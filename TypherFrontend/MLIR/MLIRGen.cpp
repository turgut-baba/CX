#include "MLIRGen.h" // TODO: change this to checker/analyzer.

namespace MLIR {

	Generator::Generator()
	{

	}

	Generator::~Generator()
	{

	}

	void Generator::Generate(ArrayAlloc<AST::Statement*>& ASTTree) {
		
		mlir::registerAsmPrinterCLOptions();
        mlir::registerMLIRContextCLOptions();

        mlir::MLIRContext context;
		builder = std::make_shared<mlir::OpBuilder>(&context);
        // Load our Dialect in this MLIR Context.
        context.getOrLoadDialect<mlir::typher::TypherDialect>();

		theModule = mlir::ModuleOp::create(builder->getUnknownLoc());

		for (AST::ASTNode* node : ASTTree.vec_)
      		node->Accept(this);

		if (failed(mlir::verify(theModule))) {
			theModule.emitError("module verification error");
			return;
		}

        theModule->dump();
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
		
		/* if (mlir::failed(mlirGen(*funcAST.getBody()))) {
      		function.erase();
      		return nullptr;
    	} */
 		mlir::typher::ReturnOp returnOp;
		if (!entryBlock.empty())
      		returnOp = dynamic_cast<mlir::typher::ReturnOp>(entryBlock.back());

		if (!returnOp) {
			mlir::typher::ReturnOp::create(*builder, location);
		} else if (returnOp.hasOperand()) {
			function.setType(builder->getFunctionType(
          		function.getFunctionType().getInputs(), builder->getI32Type()));
		}

		return ;//function;
	}

	void Generator::Visit(AST::Statement* node)
	{
		std::cout << "Statin" << std::endl;
	}

	void Generator::Visit(AST::VariableDeclarator* node) 
	{

	}
    void Generator::Visit(AST::Expression* node) 
	{

	}
	void Generator::Visit(AST::Identifier* node) 
	{

	}
	void Generator::Visit(AST::IntegerLiteral* node) 
	{

	}

	mlir::typher::FuncOp Generator::GenNode(AST::Statement* node)
	{
    	llvm::ScopedHashTableScope<llvm::StringRef, mlir::Value> varScope(symbolTable);

		builder->setInsertionPointToEnd(theModule.getBody());
		mlir::typher::FuncOp function ;//= mlirGen(*funcAST.getProto());

		std::cout << "Statin" << std::endl;

		return function;
	}

	mlir::typher::FuncOp Generator::GenNode(AST::Function* node)
	{

	}

}
