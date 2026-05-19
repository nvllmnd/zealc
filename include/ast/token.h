#pragma once

#include "nv/core/intdefs.h"

#include "nv/core/attributes.h"
#include "nv/core/sslice.h"

struct Cursor {
  /// index into whatever buffer this cursor is iterating over
  i32 i;
  /// row of item being iterated over
  i32 row;
  /// column of item being iterated over
  i32 col;
};
typedef struct Cursor Cursor;
typedef Cursor SourceLocation;

/// Constant used to shift TokenTypes joined together, so that the assigned enum is indeed unique to this enum set
/// i.e.:
/// ```c
/// typedef enum TokenType {
///     ...
///     Token__BangEq = (Token__Bang | Token__Eq) << 8 // Shift 8 to the left to completely skip over the possible asicc
///     range
//                                                     // (0-255) though i believe 0-128 is enough, so i need to
//                                                     verify...
///     ...
/// } TokenType;
/// ```
/// I have yet to try this out, but i think you could technically check if a token has a subtoken trivially this way...
/// const bool has_bang = ((token_type >> _TOKENC_SHIFT) & Token__Bang) == Token__Bang;
///
///
static constexpr const i32 _TOKENC_KEYWORDS_START = (1 << 16);
static constexpr const i32 _TOKENC_GLYPHS_START = (1 << 12);


typedef enum TokenType : i32 {
  /// A variant of this enum was passed to [token_type_glpyh_string] that
  /// falls out of range of the multi-character glyhps
  Token__CannotGetStringOfNonGlyph = -3,
  /// A variant of this enum was passed to [token_type_keyword_string] that
  /// falls out of range of the keyword TokenType constants
  Token__CannotGetStringOfNonKeyword = -2,
  /// No Token has been parsed. Lexer encountered an error, or has reached EOF
  Token__Eof = -1,
  Token__Identifier = 0,
  Token__LiteralStart,
  /// true/false
  // Token__Bool,
  /// any non-floating-point integer
  Token__Int,
  /// Any floating point integer
  Token__Float,
  /// "Any string of characters, surrouned by"
  Token__String,
  /// :colon_prefixed_unique_identifiers
  Token__Rune,
  Token__LiteralEnd,

  Token__OpenBrace = '{',
  Token__CloseBrace = '}',
  Token__OpenParen = '(',
  Token__CloseParen = ')',
  Token__OpenBracket = '[',
  Token__CloseBracket = ']',
  Token__DoubleQuot = '"',
  Token__SingleQuot = '\'',
  Token__Bang = '!',
  Token__Percent = '%',
  Token__ChevronUp = '^',
  Token__Ampersand = '&',
  Token__Star = '*',
  Token__Minus = '-',
  Token__Plus = '+',

  Token__UnaryUnderscore = '_',
  Token__Eq = '=',

  Token__Pipe = '|',
  Token__Comma = ',',
  Token__Period = '.',
  Token__Gt = '>',
  Token__Lt = '<',
  Token__Semicolon = ';',
  Token__Colon = ':',
  Token__BackSlash = '\\',
  Token__ForwardSlash = '/',
  Token__QMark = '?',

  Token__GlyphStart = _TOKENC_GLYPHS_START,

  /// *=
  Token__StarEq,  
  /// &=
  Token__AmpersandEq,  
  /// %=
  Token__PercentEq,  

  /// !=
  Token__BangEq,  
  /// ==
  Token__DoubleEq,  
  Token__PlusEq,    

  Token__MinusEq,  

  Token__DblQMark,  

  Token__DblForwardSlash,  

  Token__ForwardSlashEq,  

  Token__DoubleBackSlash,  

  Token__DoubleColon,  

  Token__LtEq,  

  Token__GtEq,  

  Token__Elipses,  

  Token__DoublePipe,  

  /// ()
  Token__EmptyParen,  
  /// []
  Token__EmptyBracket,  
  /// {}
  Token__EmptyBrace,  
  /// ->
  Token__ArrowRight,  
  /// <-
  Token__ArrowLeft,  
  /// =>
  Token__FatArrow,
  Token__ChevronEq,
  Token__PipeEq,
  Token__PipeRight,
  Token__PipeLeft,
  Token__ShiftRight,
  Token__ShiftLeft,

  Token__GlyphsEnd,
  Token__GlyphsCount = (Token__GlyphsEnd - Token__GlyphStart) - 1,

  Token__Comment,  // Token__DblForwardSlash,

  Token__KeywordsStart = _TOKENC_KEYWORDS_START,

  
  // NOTE: putting true/false as keywords, as its easier to map to
  // strings in this group as opposed to specific one for bool true/false literals
  Token__True,
  Token__False,


  Token__Let,
  Token__If,
  Token__Else,
  Token__Mut,
  Token__When,
  Token__Fn,
  Token__Struct,
  Token__Trait,
  Token__Impl,
  Token__And,
  Token__Or,
  Token__Return,
  Token__Self,
  Token__Const,
  Token__Loop,
  Token__For,
  Token__While,
  Token__Break,
  Token__Match,
  Token__Continue,
  Token__Pub,
  Token__Ref,
  Token__Error,
  Token__Enum,
  Token__Type,
  Token__Await,
  Token__Comptime,
  Token__Static,
  Token__Mod,
  Token__Macro,
  Token__Derive,
  Token__Dyn,
  Token__Default,
  Token__Sizeof,
  Token__KeywordsEnd,
  Token__KeywordCount = (Token__KeywordsEnd - Token__KeywordsStart) - 1,
  // Token__,
  // Token__,

} TokenType;


// struct Keyword { i32 id; TokenType type; };
CONST_FUNC
bool tokentype_is_keyword(TokenType self);

CONST_FUNC
bool tokentype_is_glyph(TokenType self);


/// Lex/Parse Token. If token is a integer or boolean literal, it will
/// be kept in the literal field union. If TokenType is Token__Integer, Token__Float, Token__Boolean, or
/// Token__Character then you can get the parsed literal from the literal union field
///
/// String and Rune literals are not parsed, but you can access it through the
/// lexeme field [sslice].
struct Token {
  TokenType type;
  sslice lexeme;

  /// Location in source where this token is located
  SourceLocation loc;

  /// integer or floating point literals. if any.
  /// It would be redundant to add an sslice field here for
  /// string literals, as that can be found on the lexeme field.
  /// if TokenType == Token__String, then lexeme will point to the first '"'
  /// and contain length up to the trailing '"'
  ///
  /// Same deal for runes, lexeme field will point to prefix ':'
  ///
  /// That way caller can decide if they want to copy out the lexeme
  /// if it will be needed after parsing in the resulting AST
  union {
    bool boolean;
    char character;
    i64 integer;
    f64 fp;
    // string literals can be found in lexeme field
  } literal;
};
typedef struct Token Token;
