#include "parser.h"

#include "ast.h"
#include "lexer.h"
#include "tokens.h"
#include "precedence.h"

#include <memory>

namespace enigma::parsing {


class AstBuilder {

Lexer *lexer;
ErrorHandler *herr;
Token token;

/// Parse an operand--this includes variables, literals, arrays, and
/// unary expressions on these.
std::unique_ptr<AST::Node> TryParseOperand() {
  token = lexer->ReadToken();
  switch (token.type) {
    case TT_BEGINBRACE: case TT_ENDBRACE:
    case TT_ENDPARENTH: case TT_ENDBRACKET:
    case TT_ENDOFCODE:
      return nullptr;

    case TT_SEMICOLON:
      return std::make_unique<AST::CodeBlock>();
    case TT_COLON:
      herr->ReportError(token, "Expected label or ternary expression before colon");
      token = lexer->ReadToken();
      return nullptr;
    case TT_COMMA:
      herr->ReportError(token, "Expected expression before comma");
      token = lexer->ReadToken();
      return nullptr;
    case TT_ASSIGN:
    case TT_ASSOP:
      herr->ReportError(token, "Expected assignable expression before assignment operator");
      token = lexer->ReadToken();
      return nullptr;
    case TT_DOT: case TT_ARROW:
      herr->ReportError(token, "Expected expression before member access");
      token = lexer->ReadToken();
      return nullptr;
    case TT_DOT_STAR: case TT_ARROW_STAR:
      herr->ReportError(token, "Expected expression before pointer-to-member");
      token = lexer->ReadToken();
      return nullptr;
    case TT_PERCENT: case TT_PIPE: case TT_CARET:
    case TT_AND: case TT_OR: case TT_XOR: case TT_DIV: case TT_MOD:
    case TT_EQUALS: case TT_SLASH: case TT_EQUALTO: case TT_NOTEQUAL:
    case TT_LESS: case TT_GREATER: case TT_LESSEQUAL: case TT_THREEWAY:
    case TT_GREATEREQUAL: case TT_LSH: case TT_RSH:
      herr->Error(token) << "Expected expression before binary operator `" << token.content << '`';
      token = lexer->ReadToken();
      return nullptr;

    case TT_QMARK:
      herr->Error(token) << "Expected expression before ternary operator ?";
      token = lexer->ReadToken();
      return nullptr;

    case TT_NOT: case TT_BANG: case TT_PLUS: case TT_MINUS:
    case TT_STAR: case TT_AMPERSAND: case TT_TILDE:
    case TT_INCREMENT: case TT_DECREMENT: {
      Token unary_op = token;
      token = lexer->ReadToken();
      if (auto exp = TryParseExpression(Precedence::kUnaryPrefix))
        return std::make_unique<AST::UnaryExpression>(std::move(exp), unary_op.type);
      herr->Error(unary_op) << "Expected expression following unary operator";
    }

    case TT_BEGINPARENTH: {
      token = lexer->ReadToken();
      auto exp = TryParseExpression(Precedence::kMin);
      if (token.type == TT_ENDPARENTH) {
        token = lexer->ReadToken();
      } else {
        herr->Error(token) << "Expected closing parenthesis before " << token;
      }
      return std::make_unique<AST::Parenthetical>(std::move(exp));
    }
    case TT_BEGINBRACKET: {
      token = lexer->ReadToken();
      std::vector<std::unique_ptr<AST::Node>> elements;
      while (std::unique_ptr<AST::Node> element = TryParseExpression(Precedence::kComma)) {
        elements.push_back(std::move(element));
        if (token.type != TT_COMMA) break;
        token = lexer->ReadToken();
      }
      if (token.type == TT_ENDBRACKET) {
        token = lexer->ReadToken();
      } else {
        herr->Error(token) << "Expected closing `]` for array";
      }
      return std::make_unique<AST::Array>(std::move(elements));
    }

    case TT_IDENTIFIER:
    case TT_DECLITERAL: case TT_BINLITERAL: case TT_OCTLITERAL:
    case TT_HEXLITERAL: case TT_STRINGLIT: case TT_CHARLIT:
      return std::make_unique<AST::Literal>(token);
    
    case TT_SCOPEACCESS:

    case TT_TYPE_NAME:

    case TT_LOCAL:
    case TT_GLOBAL:

    case TT_RETURN:   case TT_EXIT:   case TT_BREAK:   case TT_CONTINUE:
    case TT_S_SWITCH: case TT_S_CASE: case TT_S_DEFAULT:
    case TT_S_FOR:    case TT_S_DO:   case TT_S_WHILE: case TT_S_UNTIL:
    case TT_S_REPEAT: case TT_S_IF:   case TT_S_THEN:  case TT_S_ELSE:
    case TT_S_WITH:   case TT_S_TRY:  case TT_S_CATCH:
    case TT_S_NEW:    case TT_S_DELETE:

    case TT_CLASS:    case TT_STRUCT:
    case TTM_WHITESPACE: case TTM_CONCAT: case TTM_STRINGIFY:

    case TT_ERROR:
      return nullptr;
  }
}

std::unique_ptr<AST::Node> TryParseExpression(int precedence) {
  if (auto operand = TryParseOperand()) {
    // TODO: Handle binary operators, unary postfix operators
    // (including function calls, array indexing, etc).
    // XXX: Maybe handle TT_IDENTIFIER here when `operand` names a type
    // to parse a declaration as an expression. This is a bold move, but
    // more robust than handling it in TryParseExpression.
    (void) precedence;
    return operand;
  }
  return nullptr;
}

/// Reads if()/for()/while()/with()/switch() statements.
///
/// Syntax rule: In quirks mode, all binary operators are presumed to
/// extend a parenthetical expression, EXCLUDING the star operator but
/// INCLUDING the ampersand operator. The rationale for this is that
/// `if (expr) *stmt = ...;` is far more likely to appear in code than
/// `if (expr) &expr...;`, and `if (expr1) & (expr2)` is far more likely
/// to appear than `if (expr2) * (expr2)`. In quirks mode, use of either
/// token will result in a warning.
///
/// In strict mode, only the first parenthesized expression is taken.
///
/// In GML mode, a complete expression is read; unary * and & do not
/// exist and so are not ambiguous (same for prefix ++ and --).
template<typename ExpNode>
std::unique_ptr<ExpNode> ReadConditionalStatement() {
  
}

std::unique_ptr<AST::Node> TryReadStatement() {
  token = lexer->ReadToken();
  switch (token.type) {
    case TTM_WHITESPACE:
    case TTM_CONCAT:
    case TTM_STRINGIFY:
      herr->ReportError(token, "Internal error: Unhandled preprocessing token");
      token = lexer->ReadToken();
      return nullptr;
    case TT_ERROR:
      herr->ReportError(token, "Internal error: Bad token");
      token = lexer->ReadToken();
      return nullptr;

    case TT_COMMA:
      herr->ReportError(token, "Expected expression before comma");
      token = lexer->ReadToken();
      return nullptr;
    case TT_ENDPARENTH:
      herr->ReportError(token, "Unmatched closing parenthesis");
      token = lexer->ReadToken();
      return nullptr;
    case TT_ENDBRACKET:
      herr->ReportError(token, "Unmatched closing bracket");
      token = lexer->ReadToken();
      return nullptr;

    case TT_SEMICOLON:
      herr->ReportWarning(token, "Statement doesn't do anything (consider using `{}` instead of `;`)");
      token = lexer->ReadToken();
      return std::make_unique<AST::CodeBlock>();

    case TT_COLON: case TT_ASSIGN: case TT_ASSOP:
    case TT_DOT: case TT_ARROW: case TT_DOT_STAR: case TT_ARROW_STAR:
    case TT_PERCENT: case TT_PIPE: case TT_CARET:
    case TT_AND: case TT_OR: case TT_XOR:
    case TT_DIV: case TT_MOD: case TT_SLASH:
    case TT_EQUALS: case TT_EQUALTO: case TT_NOTEQUAL: case TT_THREEWAY:
    case TT_LESS: case TT_GREATER: case TT_LSH: case TT_RSH:
    case TT_LESSEQUAL: case TT_GREATEREQUAL:
    case TT_QMARK:
      // Allow TryParseExpression to handle errors.
      // (Fall through.)
    case TT_PLUS: case TT_MINUS: case TT_STAR: case TT_AMPERSAND:
    case TT_NOT: case TT_BANG: case TT_TILDE:
    case TT_INCREMENT: case TT_DECREMENT:
    case TT_BEGINPARENTH: case TT_BEGINBRACKET:
    case TT_DECLITERAL: case TT_BINLITERAL: case TT_OCTLITERAL:
    case TT_HEXLITERAL: case TT_STRINGLIT: case TT_CHARLIT:
    case TT_SCOPEACCESS:
      return TryParseExpression(Precedence::kAll);

    case TT_ENDBRACE:
      return nullptr;

    case TT_BEGINBRACE: {
      auto code = ParseCodeBlock();
      if (token.type != TT_ENDBRACE) {
        herr->ReportError(token, "Expected closing brace");
      }
      return code;
    }

    case TT_IDENTIFIER: {
      // TODO: Verify that template instantiations are covered by kScope
      auto name = TryParseExpression(Precedence::kScope);
      // TODO:
      // if (name.IsTypeIdentifier()) {
      //   // Handle reading decl-specifier-seq and init-declarator-list
      //   return ReadDeclaration(name);
      // } else {
      //   return TryParseBinaryExpression(name, Precedence::kAll);
      // }
      return name;
    }

    case TT_TYPE_NAME:  // TODO: rename TT_DECLARATOR, exclude var/variant/C++ classes,
                        // include cv-qualifiers, storage-specifiers, etc
    case TT_LOCAL:      // XXX: Treat as storage-specifiers?
    case TT_GLOBAL:

    case TT_RETURN: return ParseReturnStatement();
    case TT_EXIT: return ParseExitStatement();
    case TT_BREAK: return ParseBreakStatement();
    case TT_CONTINUE: return ParseContinueStatement();
    case TT_S_SWITCH: return ParseSwitchStatement();
    case TT_S_REPEAT: return ParseRepeatStatement();
    case TT_S_CASE:   return ParseCaseStatement();
    case TT_S_DEFAULT: return ParseDefaultStatement();
    case TT_S_FOR: return ParseForLoop();
    case TT_S_IF: return ParseIfStatement();
    case TT_S_DO: return ParseDoLoop();
    case TT_S_WHILE: return ParseWhileLoop();
    case TT_S_UNTIL: return ParseUntilLoop();
    case TT_S_WITH: return ParseWithStatement();

    case TT_S_THEN:
      herr->ReportError(token, "`then` statement not paired with an `if`");
      token = lexer->ReadToken();
      return nullptr;
    case TT_S_ELSE:
      herr->ReportError(token, "`else` statement not paired with an `if`");
      token = lexer->ReadToken();
      return nullptr;

    case TT_S_TRY: case TT_S_CATCH:
    case TT_S_NEW: case TT_S_DELETE:
    case TT_CLASS: case TT_STRUCT:
      herr->ReportError(token, "Internal error: Unsupported C++ keyword");
      token = lexer->ReadToken();
      return nullptr;

    case TT_ENDOFCODE: return nullptr;
    ;
  }
}

// TODO: the following.
std::unique_ptr<AST::CodeBlock> ParseCodeBlock() {
}
std::unique_ptr<AST::BinaryExpression> TryParseBinaryExpression() {
  
}
std::unique_ptr<AST::UnaryExpression> ParseUnaryExpression() {
  
}
std::unique_ptr<AST::TernaryExpression> ParseTernaryExpression() {
  
}
std::unique_ptr<AST::IfStatement> ParseIfStatement() {
  
}
std::unique_ptr<AST::ForLoop> ParseForLoop() {
  
}
std::unique_ptr<AST::WhileLoop> ParseWhileLoop() {
  
}
std::unique_ptr<AST::WhileLoop> ParseUntilLoop() {
  
}
std::unique_ptr<AST::DoLoop> ParseDoLoop() {
  
}
std::unique_ptr<AST::DoLoop> ParseRepeatStatement() {
  
}
std::unique_ptr<AST::ReturnStatement> ParseReturnStatement() {
  
}
std::unique_ptr<AST::BreakStatement> ParseBreakStatement() {
  
}
std::unique_ptr<AST::BreakStatement> ParseContinueStatement() {
  
}
std::unique_ptr<AST::ReturnStatement> ParseExitStatement() {
  
}
std::unique_ptr<AST::SwitchStatement> ParseSwitchStatement() {
  
}
std::unique_ptr<AST::CaseStatement> ParseCaseStatement() {
  
}
std::unique_ptr<AST::CaseStatement> ParseDefaultStatement() {
  
}
std::unique_ptr<AST::CaseStatement> ParseWithStatement() {
  
}

};  // class AstBuilder
}  // namespace enigma::parsing
