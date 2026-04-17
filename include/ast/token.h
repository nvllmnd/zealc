#pragma once



/// Constant used to shift TokenTypes joined together, so that the assigned enum is indeed unique to this enum set
/// i.e.:
/// ```c
/// typedef enum TokenType {
///     ...
///     Token__BangEq = (Token__Bang | Token__Eq) << 8 // Shift 8 to the left to completely skip over the possible asicc range
//                                                     // (0-255) though i believe 0-128 is enough, so i need to verify...
///     ...
/// } TokenType;
/// ```
/// I have yet to try this out, but i think you could technically check if a token has a subtoken trivially this way...
/// const bool has_bang = ((token_type >> _TOKENC_SHIFT) & Token__Bang) == Token__Bang;
///
///
#include "memory/cstr.h"
constexpr const i32 _TOKENC_SHIFT_VAL = 8;
constexpr const i32 _TOKENC_KEYWORDS_START = (1 << 16);

#define _TOKENC_JOIN(join_set) (((join_set)) << (_TOKENC_SHIFT_VAL))



typedef enum TokenType : i32 {
  Token__Identifier = 0,
  /// true/false
  Token__Bool,
  /// any non-floating-point integer
  Token__Int,
  /// Any floating point integer
  Token__Float,
  /// "Any string of characters, surrouned by"
  Token__String,
  /// :colon_prefixed_unique_identifiers
  Token__Rune,

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
  Token__Eq = '=',
  /// *=
  Token__StarEq = _TOKENC_JOIN(Token__Star | Token__Eq),
  /// &=
  Token__AmpersandEq = _TOKENC_JOIN(Token__Ampersand | Token__Eq),
  /// %=
  Token__PercentEq = _TOKENC_JOIN(Token__Percent | Token__Eq),


  /// !=
  Token__BangEq = _TOKENC_JOIN(Token__Bang | Token__Eq), 
  /// ==
  Token__DoubleEq = _TOKENC_JOIN(Token__Eq | Token__Eq),
  Token__PlusEq = _TOKENC_JOIN(Token__Plus | Token__Eq),

  Token__MinusEq = _TOKENC_JOIN(Token__Minus | Token__Eq),
  Token__QMark = '?',
  Token__DblQMark = _TOKENC_JOIN(Token__QMark | Token__QMark),
  Token__ForwardSlash = '/',
  Token__DblForwardSlash = _TOKENC_JOIN(Token__ForwardSlash | Token__ForwardSlash),

  Token__ForwardSlashEq = _TOKENC_JOIN(Token__ForwardSlash | Token__Eq),
  Token__BackSlash = '\\',
  Token__DblBackSlash = _TOKENC_JOIN(Token__BackSlash | Token__BackSlash), 
  Token__Semicolon = ';',
  Token__Colon = ':',
  Token__DblColon = _TOKENC_JOIN(Token__Colon | Token__Colon),
  Token__Lt = '<',
  Token__LtEq = _TOKENC_JOIN(Token__Lt | Token__Eq),
  Token__Gt = '>',
  Token__GtEq = _TOKENC_JOIN(Token__Gt | Token__Eq),
  Token__Comma = ',',
  Token__Period = '.',
  Token__Elipses = _TOKENC_JOIN(Token__Period | Token__Period | Token__Period),
  Token__Pipe = '|',
  Token__DblPipe = _TOKENC_JOIN(Token__Pipe | Token__Pipe),
  Token__UnaryUnderscore = '_',
  /// ()
  Token__EmptyParen = _TOKENC_JOIN(Token__OpenParen | Token__CloseParen),
  /// []
  Token__EmptyBracket = _TOKENC_JOIN(Token__OpenBracket | Token__CloseBracket),
  /// {}
  Token__EmptyBrace = _TOKENC_JOIN(Token__OpenBrace | Token__CloseBrace),
  /// ->
  Token__ArrowRight = _TOKENC_JOIN(Token__Minus | Token__Gt),
  /// <-
  Token__ArrowLeft = _TOKENC_JOIN(Token__Lt | Token__Minus),
  /// =>
  Token__FatArrow = _TOKENC_JOIN(Token__Eq | Token__Gt),

  Token__Comment = Token__DblForwardSlash,

  Token__KeywordsStart = _TOKENC_KEYWORDS_START,

} TokenType;

struct Token {
  TokenType type;
  sslice lexeme;

};
typedef struct Token Token;



