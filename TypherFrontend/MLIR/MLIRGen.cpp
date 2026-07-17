#include "MLIRGen.h" // TODO: change this to checker/analyzer.
#include "MLIRHelpers.h"
#include "Helpers.h"

namespace MLIR {
	using llvm::dyn_cast;
	using llvm::SmallVector;

	Generator::Generator()
	{
		mlir::func::registerAllExtensions(registry);
		mlir::LLVM::registerInlinerInterface(registry);

		registry.insert<mlir::arith::ArithDialect>();

		context = std::make_shared<mlir::MLIRContext>(registry);

		context->loadDialect<mlir::arith::ArithDialect>();
	}

	mlir::Value Generator::LvalueToRvalue(mlir::Value addr, mlir::Location location)
	{
		auto elementType = mlir::cast<mlir::MemRefType>(addr.getType()).getElementType();
		mlir::Value val = mlir::typher::LoadOp::create(*builder, location,
			elementType, addr);

		return val; 
	}

	Generator::~Generator()
	{

	}

	void Generator::Generate(SlabVector<AST::Statement*>& ASTTree)
	{
        context->getOrLoadDialect<mlir::typher::TypherDialect>();

		builder = std::make_shared<mlir::OpBuilder>(context.get());
		
		theModule = mlir::ModuleOp::create(builder->getUnknownLoc());

		// TODO: Add global (modular) context.
    	
		//llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);

		for (AST::ASTNode* node : ASTTree)
      		node->Accept(this);

		if (failed(mlir::verify(theModule))) {
			theModule.emitError("module verification error");
			return;
		}

        theModule->dump();
	}

	void Generator::GenBody(AST::Body* body, mlir::Location& location) 
	{
		llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);
		
		for (AST::ASTNode* child: body->Statements()) {
			child->Accept(this);
			// if (SOME ERROR HANDLING) { function.erase(); return; }
    	}
		
		if (builder->getBlock()->empty() || 
			!builder->getBlock()->back().hasTrait<mlir::OpTrait::IsTerminator>()) {
			mlir::typher::YieldOp::create(*builder, location);
		}
	}

	void Generator::Visit(AST::ExpressionStatement* node)
	{
		node->Expr()->Accept(this);
	}

	void Generator::Visit(AST::Function* node)
	{
    	llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);

		builder->setInsertionPointToEnd(theModule.getBody());

		auto location = loc(node->Loc());
		
		auto returnType = ASTTypeToMlirType(node->ReturnType(), builder);
		ApplyTypeModifiers(node->Declarator(), returnType, builder);

		auto funcArgs = node->Params();

		llvm::SmallVector<mlir::Type, 4> argTypes; // TODO: change this to accept argument types.

		for (size_t i = 0; i < funcArgs.size(); i++) 
		{
			auto arg_type = ASTTypeToMlirType(funcArgs[i].Type(), builder);
			ApplyTypeModifiers(funcArgs[i].AsDeclarator(), arg_type, builder);
			argTypes.push_back(arg_type);
		}

		auto funcType = builder->getFunctionType(argTypes, returnType);
		mlir::typher::FuncOp function = mlir::typher::FuncOp::create(*builder, location, node->Name(),
										funcType);

		mlir::Type expectedType = returnType; // TODO: change this to accept all types.

		mlir::Block &entryBlock = function.front();
		auto entryArgs = entryBlock.getArguments();

		builder->setInsertionPointToStart(&entryBlock);
		
		GenBody(node->GetBody(), location);

 		mlir::typher::ReturnOp returnOp;
		if (!entryBlock.empty()) {
			returnOp = dyn_cast<mlir::typher::ReturnOp>(entryBlock.back());
		}

		if (!returnOp) {
			// TODO: this place causes a seg fault.
			mlir::typher::ReturnOp::create(*builder, location);
		} else if(returnOp.hasOperand()){
			function.setType(builder->getFunctionType(
          		function.getFunctionType().getInputs(), returnType));
		}

		// retType = function;
	}

	void Generator::Visit(AST::Statement* node)
	{
		
	}

	void Generator::Visit(AST::VariableDeclarator* node) 
	{
		auto location = loc(node->Loc());

		mlir::Type varType = ASTTypeToMlirType(((AST::VariableDeclaration*)node->Parent())->Type(), builder);

		ApplyTypeModifiers(node, varType, builder);
		
		//auto memrefType = mlir::typher::PointerType::get(builder->getContext(), varType);
		auto memrefType = mlir::MemRefType::get({}, varType);

		mlir::Value address = mlir::typher::AllocaOp::create(*builder, 
			location, memrefType);

		if (node->Expr()) {
			node->Expr()->Accept(this);
			mlir::Value initialValue = retValue;
			
			mlir::typher::AssignOp::create(*builder, location, initialValue, address);
		}
		mlir::StringAttr persistentName = builder->getStringAttr(node->Name());

		symbolTable.insert(persistentName.getValue(), address);

		retValue = address;
	}
	
	void Generator::Visit(AST::VariableDeclaration* node) 
	{
		auto decl_list = node->Declarators();
		for(int i = 0; i < decl_list.size(); i++)
		{
			decl_list[i]->Accept(this);
			if (failed(declare(decl_list[i]->Name(), retValue)))
				return;
		}
	}
	
	void Generator::Visit(AST::Expression* node) 
	{
		node->Accept(this);

	}

	void Generator::Visit(AST::CallExpression* node) 
	{
		std::string callee = node->Callee();
		auto location = loc(node->Loc());
		// Codegen the operands first.
		SmallVector<mlir::Value, 4> operands;
		for (auto &expr : node->Args()) {
			expr->Accept(this);
			operands.push_back(retValue);
		}

		mlir::Type varType = builder->getI32Type();//ASTTypeToMlirType(node->Type(), builder);

		// TODO: do this better.
		retValue = (mlir::Value)mlir::typher::GenericCallOp::create(
				*builder, 
				location, 
				varType, // Added parameter
				callee, 
				operands
			).getResult(0);
	}

	void Generator::Visit(AST::ReturnStatement* node) 
	{
		auto location = loc(node->Loc());

		// 'return' takes an optional expression, handle that case here.
		if (node->Expr() != nullptr) {
			node->Expr()->Accept(this);
			mlir::Value addr = retValue; // This is the memref<i32> from symbolTable

			mlir::typher::ReturnOp::create(*builder, location,
				addr);
			return;
		} else {
			//retValue = NULL;
		}

		// Otherwise, this return operation has zero operands.
		mlir::typher::ReturnOp::create(*builder, location,
						retValue);
	}

	void Generator::Visit(AST::Identifier* node) 
	{
		auto location = loc(node->Loc());

		if (auto variable = symbolTable.lookup(node->Value()))
		{		
			retValue = LvalueToRvalue(variable, location);
      		//retValue = variable;
		}
		// TODO: log error
		return;
	}

	

	void Generator::Visit(AST::IntegerLiteral* node) 
	{
		if(node->IsFloating()) {
			retValue = mlir::typher::ConstantOp::create(*builder,
			 loc(node->Loc()), builder->getF32Type(), node->Value<double>());
		} else {
			retValue = mlir::typher::ConstantOp::create(*builder,
			 loc(node->Loc()), builder->getI32Type(), node->Value<int>());
		}
	}

	void Generator::Visit(AST::StringLiteral* node) 
	{
		if(node->IsChar()) {
			retValue = mlir::typher::ConstantOp::create(*builder,
			 	loc(node->Loc()), builder->getI8Type(), node->Value<char>());
		} else {
			
		}
	}

	void Generator::Visit(AST::WhileStatement* node) 
	{
		auto location = loc(node->Loc()); 

		auto whileOp = mlir::typher::WhileOp::create(*builder, location);
		mlir::Block* condBlock = builder->createBlock(&whileOp.getCondRegion());
		
		builder->setInsertionPointToStart(condBlock);
		
		node->ConditionExpr()->Accept(this);
		mlir::Value condition = retValue;
		if (!condition)
			return ;

		//builder->create<mlir::typher::ConditionYieldOp>(location, condVal);
		mlir::typher::YieldOp::create(*builder, location, condition);

		mlir::Block* bodyBlock = builder->createBlock(&whileOp.getBodyRegion());
		builder->setInsertionPointToStart(bodyBlock);
		
		GenBody(node->GetBody(), location);

		builder->setInsertionPointAfter(whileOp);
		return ; 
	}

	void Generator::Visit(AST::IfStatement* node) 
	{
		auto location = loc(node->Loc()); 
		node->ConditionExpr()->Accept(this);
		mlir::Value condition = retValue;
		if (!condition)
			return ;

		auto ifOp = mlir::typher::IfOp::create(*builder, location, condition, 
			node->HasElse() || node->HasElif());

 		builder->setInsertionPointToStart(&ifOp.getThenRegion().front());
		GenBody(node->GetBody(), location);
		
		if(node->HasElif() ) {
			builder->setInsertionPointToStart(&ifOp.getElseRegion().front());
			llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);
			node->Elif()->Accept(this);
			builder->setInsertionPointToEnd(&ifOp.getElseRegion().front());
			mlir::typher::YieldOp::create(*builder, location);
		} else if (node->HasElse()) {
			builder->setInsertionPointToStart(&ifOp.getElseRegion().front());
			GenBody(node->ElseBody(), location);
		}

		builder->setInsertionPointAfter(ifOp);
		return ; 
	}

	mlir::Value Generator::HandleRefAndAdr(AST::Operator* node)
	{
		mlir::Value address;
		if (node->OperatorType() ==  AST::OperatorKind::ADR) {
			node->GetRHS()->Accept(this);
			mlir::Value varRef = retValue;

			mlir::StringAttr persistentName = builder->getStringAttr(
				((AST::Identifier*)node->GetRHS())->Value() // TODO: Find a better approach to this.
			);
			address = symbolTable.lookup(persistentName.getValue());
			
			if (!address) {
				emitError(loc(node->Loc()), "Undefined variable target for address-of operator");
			}
		}
		return address;
	}

	void Generator::Visit(AST::Operator* node) 
	{
		auto location = loc(node->Loc());

		if(node->OperatorType() ==  AST::OperatorKind::ADR ||
		   node->OperatorType() ==  AST::OperatorKind::DRF) {
			retValue = HandleRefAndAdr(node);
			return;
		}

		node->GetRHS()->Accept(this);
		mlir::Value rhs = retValue;
		if (!rhs)
			return;

		if(node->OperatorType() ==  AST::OperatorKind::ASN)
		{
			if (auto lvalue = symbolTable.lookup(((AST::Identifier*)node->GetLHS())->Value()))
				mlir::typher::AssignOp::create(*builder, location, rhs, lvalue);
			return;
		}

		node->GetLHS()->Accept(this);
		mlir::Value lhs = retValue;
		if (!lhs)
			return;

		switch(node->OperatorType()) {
			case AST::OperatorKind::ADD: {
				retValue = mlir::arith::AddIOp::create(*builder, location, lhs, rhs);
				return;
			}
			case AST::OperatorKind::SUB: {
				retValue = mlir::arith::SubIOp::create(*builder, location, lhs, rhs);
				return;
			}
			case AST::OperatorKind::MUL: {
				retValue = mlir::arith::MulIOp::create(*builder, location, lhs, rhs);
				return;
			}
			case AST::OperatorKind::DIV: {
				retValue = mlir::arith::DivSIOp::create(*builder, location, lhs, rhs);
				return;
			}
			case AST::OperatorKind::MOD: {
				retValue = mlir::arith::RemSIOp::create(*builder, location, lhs, rhs);
				return;
			}
			default:{
				if (node->GetRHS()->Type()) { // TODO: should be decided in checker semantic analysis. The checker should decide
					     	   // which type the operation will yield via Type Promotion that uses "Path of Least Loss.".
					mlir::arith::CmpFPredicate predicate;
					switch (node->OperatorType()) {
						case AST::OperatorKind::EQS: predicate = mlir::arith::CmpFPredicate::OEQ; break;
						case AST::OperatorKind::NEQ: predicate = mlir::arith::CmpFPredicate::ONE; break;
						case AST::OperatorKind::LES: predicate = mlir::arith::CmpFPredicate::OLT; break;
						case AST::OperatorKind::LEQ: predicate = mlir::arith::CmpFPredicate::OLE; break;
						case AST::OperatorKind::GEQ: predicate = mlir::arith::CmpFPredicate::OGE; break;
						case AST::OperatorKind::GRT: predicate = mlir::arith::CmpFPredicate::OGT; break;
						default: /* handle error */ break;
					}
					retValue = mlir::arith::CmpFOp::create(*builder, location, predicate, lhs, rhs);
				} else {
					mlir::arith::CmpIPredicate predicate;
					switch (node->OperatorType()) {
						case AST::OperatorKind::EQS: predicate = mlir::arith::CmpIPredicate::eq;  break;
						case AST::OperatorKind::NEQ: predicate = mlir::arith::CmpIPredicate::ne;  break;
						case AST::OperatorKind::LES: predicate = mlir::arith::CmpIPredicate::slt; break;
						case AST::OperatorKind::LEQ: predicate = mlir::arith::CmpIPredicate::sle; break;
						case AST::OperatorKind::GEQ: predicate = mlir::arith::CmpIPredicate::sge; break;
						case AST::OperatorKind::GRT: predicate = mlir::arith::CmpIPredicate::sgt; break;
						default: /* handle error */ break;
					}
					retValue = mlir::arith::CmpIOp::create(*builder, location, predicate, lhs, rhs);
				}
				break;
			}
		}

		return;
	}
}
