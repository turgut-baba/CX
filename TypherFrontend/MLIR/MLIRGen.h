#ifndef MLIR_GEN_H
#define MLIR_GEN_H

#include "mlir/Dialect/Func/Extensions/AllExtensions.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/Transforms/InlinerInterfaceImpl.h"

#include "mlir/Dialect/Affine/Transforms/Passes.h"
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h"
#include "mlir/ExecutionEngine/ExecutionEngine.h"
#include "mlir/ExecutionEngine/OptUtils.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Export.h"
#include "mlir/Transforms/Passes.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"



#include "Dialect/TypherDialect.h"
#include "AST/Function.h"
#include "AST/Visitor.h"
#include "AST/Expressions/Operator.h"
#include "AST/Expressions/CallExpression.h"
#include "AST/statements/ReturnStatement.h"
#include "AST/statements/IfStatement.h"
#include "AST/Literals/IntegerLiteral.h"
#include "AST/statements/ExpressionStatement.h"
#include <fstream>

namespace MLIR {

	class Generator: public AST::NodeVisitor {
	public:
		Generator();
		~Generator();

		void Generate(SlabVector<AST::Statement*>& ASTTree);	

		mlir::MLIRContext& Context()
		{
			return *context.get();
		}

		mlir::ModuleOp& Module()
		{
			return theModule;
		}
	private:
		mlir::OwningOpRef<mlir::ModuleOp> GenTree();

		void Visit(AST::Function* node) override;
		void Visit(AST::Statement* node) override;
		void Visit(AST::VariableDeclarator* node) override;
        void Visit(AST::Expression* node) override;
		void Visit(AST::Identifier* node) override;
		void Visit(AST::IntegerLiteral* node) override;
		void Visit(AST::VariableDeclaration* node) override;
		void Visit(AST::Operator* node) override;
		void Visit(AST::CallExpression* node) override;
		void Visit(AST::ReturnStatement* node) override;
		void Visit(AST::IfStatement* node) override;
		void Visit(AST::ExpressionStatement* node) override;


		mlir::Location loc(const Location &loc) {
			return mlir::FileLineColLoc::get(builder->getStringAttr(loc.file), loc.line, loc.col);
		}

		llvm::LogicalResult declare(llvm::StringRef var, mlir::Value value) {
			auto persistentName = mlir::StringAttr::get(context.get(), var).getValue();
			if (symbolTable.count(persistentName))
				return mlir::failure();
			
			symbolTable.insert(persistentName, value);
			return mlir::success();
		}	

		mlir::ModuleOp theModule;
		std::shared_ptr<mlir::OpBuilder> builder;
		llvm::ScopedHashTable<llvm::StringRef, mlir::Value> symbolTable;

		mlir::Value retValue;
		
		mlir::DialectRegistry registry;
		std::shared_ptr<mlir::MLIRContext> context;
		void GenBody(AST::Body* node, mlir::Location& location);

		mlir::Value LvalueToRvalue(mlir::Value val, mlir::Location loc);
	};
}

#endif
