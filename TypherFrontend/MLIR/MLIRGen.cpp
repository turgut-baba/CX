#include "MLIRGen.h" // TODO: change this to checker/analyzer.

namespace MLIR {
	using llvm::dyn_cast;
	using llvm::SmallVector;

	Generator::Generator()
	{
		mlir::func::registerAllExtensions(registry);
		mlir::LLVM::registerInlinerInterface(registry);

		context = std::make_shared<mlir::MLIRContext>(registry);
	}

	Generator::~Generator()
	{

	}

	void Generator::Generate(SlabVector<AST::Statement*>& ASTTree) {

		
/* 		mlir::registerAsmPrinterCLOptions();
        mlir::registerMLIRContextCLOptions(); */
        context->getOrLoadDialect<mlir::typher::TypherDialect>();

		builder = std::make_shared<mlir::OpBuilder>(context.get());
		
		theModule = mlir::ModuleOp::create(builder->getUnknownLoc());


		// TODO: Add global(modular) context.
    	llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);

		for (AST::ASTNode* node : ASTTree)
      		node->Accept(this);

		if (failed(mlir::verify(theModule))) {
			theModule.emitError("module verification error");
			return;
		}

        theModule->dump();
	}

	void Generator::GenBody(AST::Statement* node) 
	{
		llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);
		
		for (AST::ASTNode* child: node->GetBody()->Statements()) {
			child->Accept(this);
			// if (SOME ERROR HANDLING) { function.erase(); return; }
    	}

	}

	void Generator::Visit(AST::Function* node)
	{
    	llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);

		builder->setInsertionPointToEnd(theModule.getBody());

		auto location = loc(node->Loc());
		llvm::SmallVector<mlir::Type, 4> argTypes(node->Params().size(),
												  builder->getI32Type()); // TODO: change this to accept all types.

		auto funcType = builder->getFunctionType(argTypes, {builder->getI32Type()});
		mlir::typher::FuncOp function = mlir::typher::FuncOp::create(*builder, location, node->Name(),
										funcType);

		mlir::Type expectedType = builder->getI32Type(); // TODO: change this to accept all types.

		mlir::Block &entryBlock = function.front();
    	auto funcArgs = node->Params();
		auto entryArgs = entryBlock.getArguments();

		for (size_t i = 0; i < funcArgs.size(); i++) 
		{
			if (entryArgs[i].getType() != expectedType) {
				return;
			}

			if (failed(declare(funcArgs[i]->Name(), entryArgs[i])))
				return;
		}

		builder->setInsertionPointToStart(&entryBlock);
		
		GenBody(node);

 		mlir::typher::ReturnOp returnOp;
		if (!entryBlock.empty()) {
			returnOp = dyn_cast<mlir::typher::ReturnOp>(entryBlock.back());
		}

		if (!returnOp) {
			// TODO: this place causes a seg fault.
			mlir::typher::ReturnOp::create(*builder, location);
		} else if(returnOp.hasOperand()){
			function.setType(builder->getFunctionType(
          		function.getFunctionType().getInputs(), builder->getI32Type()));
		}

		// retType = function;
	}

	void Generator::Visit(AST::Statement* node)
	{
	}

	void Generator::Visit(AST::VariableDeclarator* node) 
	{
		if(node->Expr() == nullptr) {
			if (auto variable = symbolTable.lookup(node->Name())) {
				retValue = variable;
				return;
			}
			// TODO: Log error, unknown variable.
		}

		node->Expr()->Accept(this);
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
		llvm::StringRef callee = node->Callee();
		auto location = loc(node->Loc());

		// Codegen the operands first.
		SmallVector<mlir::Value, 4> operands;
		for (auto &expr : node->Args()) {
			expr->Accept(this);
			operands.push_back(retValue);
		}

		// TODO: do this better.
		retValue = (mlir::Value)mlir::typher::GenericCallOp::create(*builder, 
			location, callee, operands).getResult(0);
	}

	void Generator::Visit(AST::ReturnStatement* node) 
	{
		auto location = loc(node->Loc());

		// 'return' takes an optional expression, handle that case here.
		if (node->Expr() != nullptr) {
			node->Expr()->Accept(this);
		} else {
			//retValue = NULL;
		}

		// Otherwise, this return operation has zero operands.
		mlir::typher::ReturnOp::create(*builder, location,
						retValue);
	}

	void Generator::Visit(AST::Identifier* node) 
	{
		if (auto variable = symbolTable.lookup(node->Value()))
      		retValue = variable;
		// TODO: log error
		return;
	}

	void Generator::Visit(AST::IntegerLiteral* node) 
	{
		retValue = mlir::typher::ConstantOp::create(*builder,
			 loc(node->Loc()), (int)node->Value());
	}

	void Generator::Visit(AST::IfStatement* node) 
	{
		auto location = loc(node->Loc()); 
		node->ConditionExpr()->Accept(this);
		mlir::Value condition = retValue;
		if (!condition)
			return ;

		auto ifOp = mlir::typher::IfOp::create(*builder, location, condition, 
			/*withElse=*/false);

 		builder->setInsertionPointToStart(&ifOp.getThenRegion().front());
		GenBody(node);
		
		if (builder->getBlock()->empty() || 
			!builder->getBlock()->back().hasTrait<mlir::OpTrait::IsTerminator>()) {
			mlir::typher::YieldOp::create(*builder, location );
		}	

		// 4. Fill 'else' block (if it exists)
/* 		if (node.elseBody) {
			builder.setInsertionPointToStart(&ifOp.getElseRegion().front());
			if (mlir::failed(mlirGen(*node.elseBody)))
				return mlir::failure();
			builder.create<my_dialect::YieldOp>(loc);
		}
 */
		builder->setInsertionPointAfter(ifOp);
		return ; 
	}

	void Generator::Visit(AST::Operator* node) 
	{
		node->GetLHS()->Accept(this);
		mlir::Value lhs = retValue;
		if (!lhs)
			return;

		node->GetRHS()->Accept(this);
		mlir::Value rhs = retValue;
		if (!rhs)
			return;
		auto location = loc(node->Loc());
		switch(node->OperatorType()) {
			case AST::OperatorKind::ADD: {
				retValue = mlir::typher::AddOp::create(*builder, location, lhs, rhs);
				break;
			}
			case AST::OperatorKind::EQS: {
				retValue = mlir::typher::EqualsOp::create(*builder, location, lhs, rhs);
				break;
			}
		}
	}
}
