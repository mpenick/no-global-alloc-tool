#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

namespace {
StatementMatcher CXXNewMatcher = cxxNewExpr().bind("useNew");
StatementMatcher CXXDeleteMatcher = cxxDeleteExpr().bind("useDelete");
} // namespace

class NoGlobalAllocPrinter : public MatchFinder::MatchCallback {
public :
  virtual void run(const MatchFinder::MatchResult &Result) {
    auto& Context = *Result.Context;
    auto& DiagnosticsEngine = Context.getDiagnostics();
    auto& SourceManager = Context.getSourceManager();

    const auto *NewExpr = Result.Nodes.getNodeAs<CXXNewExpr>("useNew");
    if (NewExpr &&
        !Context.getSourceManager().isInSystemHeader(NewExpr->getLocStart()) && // Not in system headers
        NewExpr->getNumPlacementArgs() == 0) { // Not placement new
      FunctionDecl* NewFuncDecl = NewExpr->getOperatorNew();
      if (NewExpr->isGlobalNew()) {
        diag(NewExpr->getLocStart(), "Using global `::operator new%0()`", DiagnosticsEngine)
            << (NewExpr->isArray() ? "[]" : "");
      } else if(NewFuncDecl &&
                Context.getSourceManager().isInSystemHeader(NewFuncDecl->getLocation())) {
        diag(NewExpr->getLocStart(), "Using `operator new%0()` from %1", DiagnosticsEngine)
            << (NewExpr->isArray() ? "[]" : "")
            << NewFuncDecl->getLocation().printToString(Context.getSourceManager());
      }
    }

    const auto *DeleteExpr = Result.Nodes.getNodeAs<CXXDeleteExpr>("useDelete");
    if (DeleteExpr &&
        !Context.getSourceManager().isInSystemHeader(DeleteExpr->getLocStart())) { // Not in system headers
      FunctionDecl* DeleteFuncDecl = DeleteExpr->getOperatorDelete();
      if (DeleteExpr->isGlobalDelete()) {
        diag(DeleteExpr->getLocStart(), "Using global `::operator delete%0()`", DiagnosticsEngine)
            << (DeleteExpr->isArrayForm() ? "[]" : "");
      } else if(DeleteFuncDecl &&
         Context.getSourceManager().isInSystemHeader(DeleteFuncDecl->getLocation())) {
        diag(DeleteExpr->getLocStart(), "Using `operator delete%0()` from %1", DiagnosticsEngine)
            << (DeleteExpr->isArrayForm()? "[]" : "")
            << DeleteFuncDecl->getLocation().printToString(Context.getSourceManager());
      }
    }
  }

  DiagnosticBuilder diag(SourceLocation Loc,
                         StringRef Description,
                         DiagnosticsEngine& DiagnosticsEngine) {
    auto ID = DiagnosticsEngine.getDiagnosticIDs()->getCustomDiagID(clang::DiagnosticIDs::Error,
                                                                    Description);
    return DiagnosticsEngine.Report(Loc, ID);
  }
};

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory NoGlobalAllocToolCategory("no-global-alloc options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nThis is a tool for find global use of operator new/delete (from either global\n"
                              "or from the standard library)\n");

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, NoGlobalAllocToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  NoGlobalAllocPrinter Printer;
  MatchFinder Finder;
  Finder.addMatcher(CXXNewMatcher, &Printer);
  Finder.addMatcher(CXXDeleteMatcher, &Printer);

  return Tool.run(newFrontendActionFactory(&Finder).get());
}
