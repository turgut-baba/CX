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

		mlir::ModuleOp theModule;
		std::shared_ptr<mlir::OpBuilder> builder;
		llvm::ScopedHashTable<llvm::StringRef, mlir::Value> symbolTable;
	};
}

#endif
