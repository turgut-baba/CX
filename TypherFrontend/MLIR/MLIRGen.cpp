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
		mlir::Type addrType = addr.getType();
		
		if (auto memrefType = mlir::dyn_cast<mlir::MemRefType>(addrType)) {
			mlir::Type elementType = memrefType.getElementType();
			return mlir::typher::LoadOp::create(*builder, location, elementType, addr);
		} 

		if (auto ptrType = mlir::dyn_cast<mlir::typher::PointerType>(addrType)) {
        	mlir::Type elementType = ptrType.getElementType();
        	return builder->create<mlir::typher::LoadOp>(location, elementType, addr);
    	}

		if(auto arrayType = mlir::dyn_cast<mlir::typher::ArrayType>(addrType)) {
			// TODO: This doesn't work because the array variables are not allocated as ArrayType.
			mlir::Type elementType = arrayType.getElementType();
			return builder->create<mlir::typher::LoadOp>(location, elementType, addr);
		}

		return addr;
	}

	mlir::Value Generator::GenArrayAccess(AST::MemoryOperation* node, mlir::Type array_type)
	{
		llvm::SmallVector<mlir::Value, 4> indexValues;
		mlir::StringAttr persistentName = builder->getStringAttr(
			((AST::Identifier*)node->GetExpression())->Value() // TODO: Find a better approach to this.
		);
		
		mlir::Value address = symbolTable.lookup(persistentName.getValue());
		// Check if base address points to an ArrayType (!typher.ptr<!typher.array<...>>)
		auto ptrType = mlir::dyn_cast<mlir::typher::PointerType>(address.getType());
		if (ptrType && mlir::isa<mlir::typher::ArrayType>(ptrType.getElementType())) {
			// Prepend index 0 to dereference the array pointer
			mlir::Value zeroIdx = builder->create<mlir::arith::ConstantIndexOp>(loc(node->Loc()), 0);
			indexValues.push_back(zeroIdx);
		}

		for (AST::Expression* indexExpr : node->ArrayIndices()) {
			indexExpr->Accept(this);
			mlir::Value indexVal = retValue;
			
			if (!indexVal.getType().isIndex()) {
				indexVal = builder->create<mlir::arith::IndexCastOp>(
					loc(indexExpr->Loc()), 
					builder->getIndexType(), 
					indexVal
				);
			}
			indexValues.push_back(indexVal);
		}

		auto resultPtrType = mlir::typher::PointerType::get(builder->getContext(), array_type);

		// address is guaranteed to be !typher.ptr<!typher.array<3 x i32>>
		address = builder->create<mlir::typher::AccessOp>(
			loc(node->Loc()),
			resultPtrType, // !typher.ptr<i32>
			address,       // !typher.ptr<!typher.array<3 x i32>>
			indexValues    // [%c0, %c1]
		);

		return address;
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
		
		mlir::Type returnType = ASTTypeToMlirType(node->ReturnType(), builder);
		ApplyTypeModifiers(node->Declarator(), returnType, builder);

		auto funcArgs = node->Params();

		llvm::SmallVector<mlir::Type, 4> argTypes;

		for (size_t i = 0; i < funcArgs.size(); i++) 
		{
			auto arg_type = ASTTypeToMlirType(funcArgs[i].Type(), builder);
			ApplyTypeModifiers(funcArgs[i].AsDeclarator(), arg_type, builder);
			
			argTypes.push_back(arg_type);
		}

		mlir::FunctionType funcType = builder->getFunctionType(argTypes, returnType);
		mlir::typher::FuncOp function = mlir::typher::FuncOp::create(*builder, location, node->Name(),
										funcType);

		mlir::Type expectedType = returnType;

		mlir::Block &entryBlock = function.front();
		auto entryArgs = entryBlock.getArguments();

		builder->setInsertionPointToStart(&entryBlock);

		for (size_t i = 0; i < funcArgs.size(); ++i) {
			mlir::StringAttr paramName = builder->getStringAttr(funcArgs[i].Name());
			mlir::Value mlirArg = entryArgs[i]; // The incoming Rvalue argument
			
			// 1. Get the type of the argument
			mlir::Type argType = mlirArg.getType();

			// 2. Allocate a local stack slot for the parameter (Lvalue)
			// Note: If using MemRefType for stack slots, build memref<argType>
			// auto memrefType = mlir::MemRefType::get({}, argType);
			auto memrefType = mlir::typher::PointerType::get(builder->getContext(), argType);
			mlir::Value paramAlloc = builder->create<mlir::typher::AllocaOp>(location, memrefType);

			// 3. Store the incoming argument value into the stack slot
			builder->create<mlir::typher::AssignOp>(location, mlirArg, paramAlloc);

			// 4. Register the STACK SLOT (Lvalue) in the symbol table, NOT the raw argument
			symbolTable.insert(paramName.getValue(), paramAlloc);
		}

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
		UNREACHABLE("Pure statment reached on MLIRGen.");
	}

	void Generator::Visit(AST::VariableDeclarator* node) 
	{
		auto location = loc(node->Loc());

		mlir::Type varType = ASTTypeToMlirType(((AST::VariableDeclaration*)node->Parent())->Type(), builder);

		ApplyTypeModifiers(node, varType, builder);
		
		auto memrefType = mlir::typher::PointerType::get(builder->getContext(), varType);
		//auto memrefType = mlir::MemRefType::get({}, varType);

		mlir::Value address = mlir::typher::AllocaOp::create(*builder, 
			location, memrefType);

		if (node->Expr()) {
			node->Expr()->Accept(this);
			mlir::Value initialValue = retValue;
			
			mlir::typher::AssignOp::create(*builder, location, initialValue, address);
		}
		
		mlir::StringAttr varName = builder->getStringAttr(node->Name());
		symbolTable.insert(varName.getValue(), address);

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
			addr = LvalueToRvalue(addr, location);

			mlir::typher::ReturnOp::create(*builder, location, addr);
		} else {
			mlir::typher::ReturnOp::create(*builder, location,
				retValue);
		}
	}

	void Generator::Visit(AST::InitializerList* node)
	{
		auto location = loc(node->Loc());

		const auto &elements = node->GetElements();
		size_t count = elements.size();

		if (count == 0) {
			// TODO: Handle empty initializer list {} if supported
			return;
		}

		// 1. Determine element type (e.g. i32)
		mlir::Type elemType = builder->getI32Type(); 

		// 2. Build aggregate array type: !typher.array<N x T>
		auto arrayType = mlir::typher::ArrayType::get(
			builder->getContext(), count, elemType
		);

		// 3. Allocate stack slot for this compound aggregate
		// Result type: !typher.ptr<!typher.array<N x T>>
		auto ptrToArrayType = mlir::typher::PointerType::get(
			builder->getContext(), arrayType
		);
		
		mlir::Value tempAlloc = builder->create<mlir::typher::AllocaOp>(
			location, 
			ptrToArrayType
		);

		// 4. Element pointer type for GEP: !typher.ptr<T>
		auto elemPtrType = mlir::typher::PointerType::get(
			builder->getContext(), elemType
		);

		// Constant index 0 needed for outer pointer dereference
		mlir::Value zeroIdx = builder->create<mlir::arith::ConstantIndexOp>(
			location, 0
		);

		// 5. Unified Loop: Evaluate and store all elements [0..N-1]
		for (size_t i = 0; i < count; ++i) {
			// Evaluate element expression -> sets retValue
			elements[i]->Accept(this);
			mlir::Value valToStore = retValue;

			llvm::SmallVector<mlir::Value, 4> indexValues;

			mlir::Value elemIdx = builder->create<mlir::arith::ConstantIndexOp>(
				location, i
			);

			indexValues.push_back(zeroIdx);
			indexValues.push_back(elemIdx);

			// ✅ FIX: Pass TWO indices {%c0, %i} so AccessOp dereferences 
			// !typher.ptr<!typher.array<N x T>> to !typher.ptr<T>
			mlir::Value elemPtr = builder->create<mlir::typher::AccessOp>(
				location, 
				elemPtrType, 
				tempAlloc, 
				indexValues // <--- Prepend zeroIdx here!
			);

			// Store value into element address
			builder->create<mlir::typher::StoreOp>(
				location, valToStore, elemPtr
			);
		}

		// Return pointer to temporary array aggregate
		retValue = tempAlloc;
	}


	void Generator::Visit(AST::MemoryOperation* node) 
	{
		//node->GetExpression()->Accept(this);
		mlir::Value address = retValue;
		
		for (int i = 0; i < node->AddressDepth(); i++) {
			mlir::StringAttr persistentName = builder->getStringAttr(
				((AST::Identifier*)node->GetExpression())->Value() // TODO: Find a better approach to this.
			);
			address = symbolTable.lookup(persistentName.getValue());
			
			if (!address) {
				emitError(loc(node->Loc()), "Undefined variable target for address-of operator");
			}
		}

		if (!node->ArrayIndices().empty()) 
		{
			mlir::Type mlirElemType = builder->getI32Type(); // TODO: change this.

			address = GenArrayAccess(node, mlirElemType);

			//address = LvalueToRvalue(address, loc(node->Loc()));
		}

		for (int i = 0; i < node->DeRefDepth(); i++) { 
			std::cout << "Deref depth: " << i << std::endl;
			address = LvalueToRvalue(address, loc(node->Loc()));
		}

		retValue = address;
	}

	void Generator::Visit(AST::Identifier* node) 
	{
		auto location = loc(node->Loc());

		if (auto variable = symbolTable.lookup(node->Value()))
		{
			mlir::Type varType = variable.getType();

			// Check if the symbol is a memory allocation (MemRef or Typher Pointer)
			retValue = variable; 
			
			return;
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

	void Generator::Visit(AST::ForStatement* node) 
	{
		auto location = loc(node->Loc()); 

		if (node->InitializeStmt()) {
			node->InitializeStmt()->Accept(this);
		}

		auto whileOp = mlir::typher::WhileOp::create(*builder, location);
		
		mlir::Block* condBlock = builder->createBlock(&whileOp.getCondRegion());
		builder->setInsertionPointToStart(condBlock);
		
		if (node->ConditionExpr()) {
			node->ConditionExpr()->Accept(this);
		} else {
			// If the condition is empty (e.g., for(;;)), it's an infinite loop. 
			// Materialize a constant true boolean (i1).
			retValue = builder->create<mlir::arith::ConstantIntOp>(location, 1, 1);
		}
		
		mlir::Value condition = retValue;
		if (!condition)
			return;


		if (node->IteratorExpr()) {
			node->IteratorExpr()->Accept(this);
		}

		mlir::typher::YieldOp::create(*builder, location, condition);

		mlir::Block* bodyBlock = builder->createBlock(&whileOp.getBodyRegion());
		builder->setInsertionPointToStart(bodyBlock);
		
		// Generate code for the actual loop statements
		GenBody(node->GetBody(), location);

		builder->setInsertionPointAfter(whileOp);
		return;
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

	void Generator::Visit(AST::Operator* node) 
	{
		auto location = loc(node->Loc());

		node->GetRHS()->Accept(this);
		mlir::Value rhs = LvalueToRvalue(retValue, location);

		if (!rhs)
			return;

		if(node->OperatorType() ==  AST::OperatorKind::ASN)
		{
			node->GetLHS()->Accept(this);
			mlir::Value lvalue = retValue;

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
