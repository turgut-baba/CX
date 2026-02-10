#include "MLIRGen.h" // TODO: change this to checker/analyzer.

namespace MLIR {
	using llvm::dyn_cast;

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
		} else if (returnOp.hasOperand()) {
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

		// TODO: This could be any expression.
		AST::Operator* Operation = (AST::Operator*)node->Expr(); // TODO: Could be standalone literal not an operation.
		Visit(Operation);
	}
	
	void Generator::Visit(AST::VariableDeclaration* node) 
	{
		std::cout << "Declared" << std::endl;
		auto decl_list = node->Declarators();
		for(int i = 0; i < decl_list.size(); i++)
		{
			Visit(decl_list[i]);
			if (failed(declare(decl_list[i]->Name(), retValue)))
				return;
		}
	}
	
	void Generator::Visit(AST::Expression* node) 
	{
		if (AST::CallExpression* c1 = dynamic_cast<AST::CallExpression*>(node)) {
			std::cout << "CallExpression" << std::endl;
		} else if (AST::Operator* c2 = dynamic_cast<AST::Operator*>(node)) {
			Visit((AST::Operator*)node);
		} else if (AST::VariableDeclarator* c2 = dynamic_cast<AST::VariableDeclarator*>(node)) {
			std::cout << "VariableDeclarator" << std::endl;
		}
		std::cout << "Expressin" << std::endl;
	}

	void Generator::Visit(AST::Identifier* node) 
	{
		
	}

	void Generator::Visit(AST::IntegerLiteral* node) 
	{
		retValue = mlir::typher::ConstantOp::create(*builder,
			 loc(node->Loc()), node->Value());
	}

	void Generator::Visit(AST::Operator* node) 
	{
		Visit((AST::IntegerLiteral*)node->GetLHS()); // TODO: Fix this
		mlir::Value lhs = retValue;
		if (!lhs)
			return;

		Visit((AST::IntegerLiteral*)node->GetRHS()); // TODO: Fix this
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

	mlir::typher::FuncOp Generator::GenNode(AST::Statement* node)
	{

	}

	mlir::typher::FuncOp Generator::GenNode(AST::Function* node)
	{

	}

}
