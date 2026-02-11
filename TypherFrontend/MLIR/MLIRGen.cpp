#include "MLIRGen.h" // TODO: change this to checker/analyzer.

namespace MLIR {
	using llvm::dyn_cast;
	using llvm::SmallVector;

	Generator::Generator()
	{

	}

	Generator::~Generator()
	{

	}

	void Generator::Generate(ArrayAlloc<AST::Statement*>& ASTTree) {
		
		mlir::registerAsmPrinterCLOptions();
        mlir::registerMLIRContextCLOptions();

		builder = std::make_shared<mlir::OpBuilder>(&context);
        // Load our Dialect in this MLIR Context.
        context.getOrLoadDialect<mlir::typher::TypherDialect>();

		theModule = mlir::ModuleOp::create(builder->getUnknownLoc());

		// TODO: Add global(modular) context.
    	//llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);

		for (AST::ASTNode* node : ASTTree.vec_)
      		node->Accept(this);

		if (failed(mlir::verify(theModule))) {
			theModule.emitError("module verification error");
			return;
		}

        theModule->dump();
	}

	void Generator::GenFunctionBody(AST::Function* node) 
	{
		llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);
		for (AST::ASTNode* child: node->Chlidren()) {
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

		auto funcType = builder->getFunctionType(argTypes, {});
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
		
		GenFunctionBody(node);
 		mlir::typher::ReturnOp returnOp;
		if (!entryBlock.empty())
      		returnOp = dyn_cast<mlir::typher::ReturnOp>(entryBlock.back());

		if (!returnOp) {
			mlir::typher::ReturnOp::create(*builder, location);
		} else {
			function.setType(builder->getFunctionType(
          		function.getFunctionType().getInputs(), builder->getI32Type()));
		}
	}

	void Generator::Visit(AST::Statement* node)
	{
		std::cout << "Statin" << std::endl;
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
		std::cout << "Declared" << std::endl;
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
		if (AST::CallExpression* c1 = dynamic_cast<AST::CallExpression*>(node)) {
			Visit((AST::CallExpression*)node);
		} else if (AST::Operator* c2 = dynamic_cast<AST::Operator*>(node)) {
			Visit((AST::Operator*)node);
		} else if (AST::VariableDeclarator* c2 = dynamic_cast<AST::VariableDeclarator*>(node)) {
			std::cout << "VariableDeclarator" << std::endl;
		}
		std::cout << "Expressin" << std::endl;
	}

	void Generator::Visit(AST::CallExpression* node) 
	{
		std::cout << "Startin call expr" << std::endl;
		llvm::StringRef callee = node->Callee();
		auto location = loc(node->Loc());

		// Codegen the operands first.
		SmallVector<mlir::Value, 4> operands;
		for (auto &expr : node->Args().vec_) {
			expr->Accept(this);
			operands.push_back(retValue);
		}

		// Otherwise this is a call to a user-defined function. Calls to
		// user-defined functions are mapped to a custom call that takes the callee
		// name as an attribute.
		retValue = mlir::typher::GenericCallOp::create(*builder, location, callee, operands);
	}

	void Generator::Visit(AST::ReturnStatement* node) 
	{
		auto location = loc(node->Loc());

		// 'return' takes an optional expression, handle that case here.
		bool has_expr = false;
		if (node->Expr() != nullptr) {
			node->Expr()->Accept(this);
			has_expr = true;
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

	void Generator::Visit(AST::Operator* node) 
	{
		std::cout << "Aight op" << std::endl;
		node->GetLHS()->Accept(this);
		std::cout << "Acceptin" << std::endl;
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
			}
		}
	}

}
