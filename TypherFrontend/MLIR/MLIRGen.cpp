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
		mlir::typher::FuncOp function ;//= mlirGen(*funcAST.getProto());

		std::cout << "Funkin" << std::endl;

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
