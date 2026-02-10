#ifndef MLIR_GEN_H
#define MLIR_GEN_H

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "mlir/IR/AsmState.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/ADT/StringRef.h"

#include "Dialect/TypherDialect.h"
#include "AST/Function.h"
#include "AST/Visitor.h"
#include "AST/Expressions/Operator.h"
#include "AST/Expressions/CallExpression.h"
#include "AST/Literals/IntegerLiteral.h"


#include <fstream>

namespace MLIR {

	class Generator: public AST::NodeVisitor {
	public:
		Generator();
		~Generator();

		void Generate(ArrayAlloc<AST::Statement*>& ASTTree);
	
	private:
		mlir::OwningOpRef<mlir::ModuleOp> GenTree();

		mlir::typher::FuncOp GenNode(AST::Function* node);
		mlir::typher::FuncOp GenNode(AST::Statement* node);

		void Visit(AST::Function* node) override;
		void Visit(AST::Statement* node) override;
		void Visit(AST::VariableDeclarator* node) override;
        void Visit(AST::Expression* node) override;
		void Visit(AST::Identifier* node) override;
		void Visit(AST::IntegerLiteral* node) override;
		void Visit(AST::VariableDeclaration* node) override;
		void Visit(AST::Operator* node) override;

		mlir::Location loc(const Location &loc) {
			return mlir::FileLineColLoc::get(builder->getStringAttr(loc.file), loc.line, loc.col);
		}

		llvm::LogicalResult declare(llvm::StringRef var, mlir::Value value) {
			auto persistentName = mlir::StringAttr::get(&context, var).getValue();
			if (symbolTable.count(persistentName))
				return mlir::failure();
			
			symbolTable.insert(persistentName, value);
			return mlir::success();
		}	

		mlir::ModuleOp theModule;
		std::shared_ptr<mlir::OpBuilder> builder;
		llvm::ScopedHashTable<llvm::StringRef, mlir::Value> symbolTable;

		mlir::Value retValue;
		mlir::MLIRContext context;
		void GenFunctionBody(AST::Function* node);
	};
}

#endif
