#ifndef MLIR_GEN_H
#define MLIR_GEN_H

#include "Parser.h" // TODO: change this to checker/analyzer.
#include "MLIRBuilder.h"
#include "AST/Function.h"

#include <fstream>

namespace MLIR {
	class ModuleOp;

	class Generator {
	public:
		Generator();
		~Generator();

		//Generator(mlir::MLIRContext& context) : builder(&context) {}

		void Generate(ArrayAlloc<AST::Statement*>& ASTTree);

		std::ofstream& GetFile();

		void Print() {
			Builder bldr = Builder();
			bldr.BuildModule();


			file << "}";
			file.flush(); // ensure data is written

			std::ifstream in("test.mlir");
			std::string line;
			while (std::getline(in, line)) {
				std::cout << line << '\n';
			}
		}
	private:
		template<typename Str>
		void AddLine(Str str, bool new_line = true)
		{
			for (int i = 0; i < ident; i++)
			{
				file << "   ";
			}

			file << str;

			if (new_line)
				file << "\n";
		}

		std::ofstream file;

		//mlir::ModuleOp Module_;

		void Gen(AST::Function* node);
		int ident = 0;
		//void Gen(AST::Operator* node);
		//void Gen(AST::Statement* node);
		//void Gen(AST::Expression* node);
	};
}

#endif