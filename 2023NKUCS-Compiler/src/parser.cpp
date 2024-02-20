/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */
#line 1 "src/parser.y"

    #include <iostream>
    #include <assert.h>
    #include <vector>
    #include <stack>
    #include "parser.h"
    extern Ast ast;
    int yylex();
    int yyerror( char const * );
    Type *curType = nullptr;
    int useforwhilecheck=0;//用于类型检查
    std::vector<int> arr_indices;
    std::vector<int> decl_arr_indices;


    ExprNode **initArray = nullptr; // 这个存储当前数组的初始化数组的基地址
    int idx; // 这个是上边那个initArray的索引
    std::stack<std::vector<int>> dimesionStack; // 维度栈

    int spillPos = 1; // 这个记录当前的参数的溢出位置
    int intArgNum = 0; // 这个记录的是当前函数参数是第几个参数，因为前四个参数用寄存器存，之后的参数用栈传递
    int floatArgNum = 0;

#line 92 "src/parser.cpp"




# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ID = 3,                         /* ID  */
  YYSYMBOL_INTEGER = 4,                    /* INTEGER  */
  YYSYMBOL_FLOATNUMBER = 5,                /* FLOATNUMBER  */
  YYSYMBOL_IF = 6,                         /* IF  */
  YYSYMBOL_ELSE = 7,                       /* ELSE  */
  YYSYMBOL_WHILE = 8,                      /* WHILE  */
  YYSYMBOL_INT = 9,                        /* INT  */
  YYSYMBOL_VOID = 10,                      /* VOID  */
  YYSYMBOL_FLOAT = 11,                     /* FLOAT  */
  YYSYMBOL_LPAREN = 12,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 13,                    /* RPAREN  */
  YYSYMBOL_LBRACE = 14,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 15,                    /* RBRACE  */
  YYSYMBOL_LBRACKET = 16,                  /* LBRACKET  */
  YYSYMBOL_RBRACKET = 17,                  /* RBRACKET  */
  YYSYMBOL_SEMICOLON = 18,                 /* SEMICOLON  */
  YYSYMBOL_ADD = 19,                       /* ADD  */
  YYSYMBOL_SUB = 20,                       /* SUB  */
  YYSYMBOL_MUL = 21,                       /* MUL  */
  YYSYMBOL_DIV = 22,                       /* DIV  */
  YYSYMBOL_MOD = 23,                       /* MOD  */
  YYSYMBOL_LESS = 24,                      /* LESS  */
  YYSYMBOL_GREATER = 25,                   /* GREATER  */
  YYSYMBOL_LESSEQL = 26,                   /* LESSEQL  */
  YYSYMBOL_GREATEREQL = 27,                /* GREATEREQL  */
  YYSYMBOL_EQL = 28,                       /* EQL  */
  YYSYMBOL_NOTEQL = 29,                    /* NOTEQL  */
  YYSYMBOL_AND = 30,                       /* AND  */
  YYSYMBOL_OR = 31,                        /* OR  */
  YYSYMBOL_NOT = 32,                       /* NOT  */
  YYSYMBOL_ASSIGN = 33,                    /* ASSIGN  */
  YYSYMBOL_RETURN = 34,                    /* RETURN  */
  YYSYMBOL_CONST = 35,                     /* CONST  */
  YYSYMBOL_COMMA = 36,                     /* COMMA  */
  YYSYMBOL_BREAK = 37,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 38,                  /* CONTINUE  */
  YYSYMBOL_THEN = 39,                      /* THEN  */
  YYSYMBOL_YYACCEPT = 40,                  /* $accept  */
  YYSYMBOL_Program = 41,                   /* Program  */
  YYSYMBOL_Stmts = 42,                     /* Stmts  */
  YYSYMBOL_Stmt = 43,                      /* Stmt  */
  YYSYMBOL_Type = 44,                      /* Type  */
  YYSYMBOL_ExprStmt = 45,                  /* ExprStmt  */
  YYSYMBOL_LVal = 46,                      /* LVal  */
  YYSYMBOL_ArrayIndices = 47,              /* ArrayIndices  */
  YYSYMBOL_AssignStmt = 48,                /* AssignStmt  */
  YYSYMBOL_BlockStmt = 49,                 /* BlockStmt  */
  YYSYMBOL_50_1 = 50,                      /* $@1  */
  YYSYMBOL_NullStmt = 51,                  /* NullStmt  */
  YYSYMBOL_IfStmt = 52,                    /* IfStmt  */
  YYSYMBOL_WhileStmt = 53,                 /* WhileStmt  */
  YYSYMBOL_54_2 = 54,                      /* $@2  */
  YYSYMBOL_BreakStmt = 55,                 /* BreakStmt  */
  YYSYMBOL_ContinueStmt = 56,              /* ContinueStmt  */
  YYSYMBOL_ReturnStmt = 57,                /* ReturnStmt  */
  YYSYMBOL_Exp = 58,                       /* Exp  */
  YYSYMBOL_Cond = 59,                      /* Cond  */
  YYSYMBOL_PrimaryExp = 60,                /* PrimaryExp  */
  YYSYMBOL_FuncRParams = 61,               /* FuncRParams  */
  YYSYMBOL_UnaryExp = 62,                  /* UnaryExp  */
  YYSYMBOL_MULExp = 63,                    /* MULExp  */
  YYSYMBOL_AddExp = 64,                    /* AddExp  */
  YYSYMBOL_RelExp = 65,                    /* RelExp  */
  YYSYMBOL_EqlExp = 66,                    /* EqlExp  */
  YYSYMBOL_LAndExp = 67,                   /* LAndExp  */
  YYSYMBOL_LOrExp = 68,                    /* LOrExp  */
  YYSYMBOL_DeclStmt = 69,                  /* DeclStmt  */
  YYSYMBOL_VarDeclStmt = 70,               /* VarDeclStmt  */
  YYSYMBOL_ConstDeclStmt = 71,             /* ConstDeclStmt  */
  YYSYMBOL_VarDefList = 72,                /* VarDefList  */
  YYSYMBOL_ConstDefList = 73,              /* ConstDefList  */
  YYSYMBOL_DeclArrayIndices = 74,          /* DeclArrayIndices  */
  YYSYMBOL_VarDef = 75,                    /* VarDef  */
  YYSYMBOL_76_3 = 76,                      /* @3  */
  YYSYMBOL_ConstDef = 77,                  /* ConstDef  */
  YYSYMBOL_78_4 = 78,                      /* @4  */
  YYSYMBOL_InitVal = 79,                   /* InitVal  */
  YYSYMBOL_80_5 = 80,                      /* $@5  */
  YYSYMBOL_InitValList = 81,               /* InitValList  */
  YYSYMBOL_ConstInitVal = 82,              /* ConstInitVal  */
  YYSYMBOL_83_6 = 83,                      /* $@6  */
  YYSYMBOL_ConstInitValList = 84,          /* ConstInitValList  */
  YYSYMBOL_FuncFParam = 85,                /* FuncFParam  */
  YYSYMBOL_FuncFParams = 86,               /* FuncFParams  */
  YYSYMBOL_FuncDef = 87,                   /* FuncDef  */
  YYSYMBOL_88_7 = 88,                      /* $@7  */
  YYSYMBOL_89_8 = 89                       /* $@8  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  59
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   242

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  40
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  108
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  182

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   294


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    74,    74,    79,    80,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    98,   101,   104,   111,
     116,   130,   147,   150,   158,   164,   163,   175,   181,   188,
     191,   197,   197,   206,   217,   228,   233,   242,   247,   252,
     255,   259,   264,   269,   272,   276,   283,   285,   287,   292,
     297,   319,   321,   332,   343,   351,   353,   365,   380,   382,
     388,   393,   398,   406,   408,   413,   421,   423,   432,   434,
     445,   446,   450,   463,   476,   480,   487,   491,   497,   500,
     507,   520,   533,   554,   554,   592,   613,   613,   656,   663,
     663,   685,   697,   699,   702,   708,   716,   716,   739,   740,
     748,   772,   785,   808,   812,   817,   824,   832,   824
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ID", "INTEGER",
  "FLOATNUMBER", "IF", "ELSE", "WHILE", "INT", "VOID", "FLOAT", "LPAREN",
  "RPAREN", "LBRACE", "RBRACE", "LBRACKET", "RBRACKET", "SEMICOLON", "ADD",
  "SUB", "MUL", "DIV", "MOD", "LESS", "GREATER", "LESSEQL", "GREATEREQL",
  "EQL", "NOTEQL", "AND", "OR", "NOT", "ASSIGN", "RETURN", "CONST",
  "COMMA", "BREAK", "CONTINUE", "THEN", "$accept", "Program", "Stmts",
  "Stmt", "Type", "ExprStmt", "LVal", "ArrayIndices", "AssignStmt",
  "BlockStmt", "$@1", "NullStmt", "IfStmt", "WhileStmt", "$@2",
  "BreakStmt", "ContinueStmt", "ReturnStmt", "Exp", "Cond", "PrimaryExp",
  "FuncRParams", "UnaryExp", "MULExp", "AddExp", "RelExp", "EqlExp",
  "LAndExp", "LOrExp", "DeclStmt", "VarDeclStmt", "ConstDeclStmt",
  "VarDefList", "ConstDefList", "DeclArrayIndices", "VarDef", "@3",
  "ConstDef", "@4", "InitVal", "$@5", "InitValList", "ConstInitVal", "$@6",
  "ConstInitValList", "FuncFParam", "FuncFParams", "FuncDef", "$@7", "$@8", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-160)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-107)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     204,    37,  -160,  -160,     4,    32,  -160,  -160,  -160,   114,
      -2,  -160,   114,   114,   114,     7,    84,    34,    57,    50,
     204,  -160,    83,  -160,    70,  -160,  -160,  -160,  -160,  -160,
    -160,  -160,  -160,    86,  -160,  -160,   101,    43,  -160,  -160,
    -160,  -160,   114,   114,    91,   114,   114,  -160,    98,  -160,
     204,  -160,  -160,  -160,  -160,    95,   111,  -160,  -160,  -160,
    -160,     2,    24,  -160,   114,  -160,   114,   114,   114,   114,
     114,  -160,    -8,   103,   114,   112,    43,    64,    28,    97,
      99,   116,  -160,   168,  -160,    21,    25,  -160,   114,   114,
      31,   120,  -160,   133,   119,  -160,  -160,  -160,   101,   101,
    -160,   114,  -160,   121,   204,   114,   114,   114,   114,   114,
     114,   114,   114,  -160,  -160,   114,    65,  -160,   111,   123,
    -160,   114,  -160,    84,    69,  -160,  -160,  -160,  -160,   135,
      43,    43,    43,    43,    64,    64,    28,    97,   204,  -160,
    -160,  -160,  -160,   127,    26,   142,  -160,    -4,   204,  -160,
      96,  -160,   132,  -160,  -160,   134,  -160,    84,  -160,   136,
    -160,  -160,  -160,    26,   131,   138,  -160,  -160,    96,  -160,
       0,   137,  -160,  -160,     5,  -160,    26,    91,  -160,    96,
    -160,  -160
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    20,    40,    41,     0,     0,    16,    18,    17,     0,
      25,    28,     0,     0,     0,     0,     0,     0,     0,     0,
       2,     3,     0,    12,    39,     5,     6,    13,     7,     8,
      15,    14,     9,     0,    46,    51,    55,    37,    10,    70,
      71,    11,    45,     0,    21,     0,     0,    39,     0,    27,
       0,    47,    48,    49,    36,     0,     0,    33,    34,     1,
       4,    80,     0,    74,     0,    19,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,    58,    63,    66,    68,
      38,     0,    42,     0,    35,     0,     0,    76,     0,     0,
      82,     0,    72,     0,     0,    52,    53,    54,    56,    57,
      50,     0,    22,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    31,    26,     0,     0,    73,     0,     0,
      81,     0,    83,   105,    80,    75,    24,    44,    23,    29,
      59,    61,    60,    62,    64,    65,    67,    69,     0,    85,
      86,    77,    78,     0,     0,     0,   103,     0,     0,    32,
       0,    79,    89,    88,    84,   100,   107,     0,    30,    96,
      94,    87,    91,     0,     0,     0,   104,    95,     0,    92,
       0,   101,   108,    98,     0,    90,     0,   102,    97,     0,
      93,    99
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -160,  -160,   105,   -17,   -14,  -160,     1,   -15,  -160,    -7,
    -160,  -160,  -160,  -160,  -160,  -160,  -160,  -160,    -9,   115,
    -160,  -160,    10,    13,   -38,   -13,    46,    48,  -160,  -160,
    -160,  -160,  -160,  -160,    77,    71,  -160,    45,  -160,  -159,
    -160,  -160,  -120,  -160,  -160,     8,  -160,  -160,  -160,  -160
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    19,    20,    21,    22,    23,    47,    44,    25,    26,
      50,    27,    28,    29,   138,    30,    31,    32,    33,    75,
      34,    72,    35,    36,    37,    77,    78,    79,    80,    38,
      39,    40,    62,    86,    90,    63,   144,    87,   150,   154,
     163,   170,   161,   168,   174,   146,   147,    41,    91,   165
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      48,    24,    56,    60,   169,   100,    55,    76,    76,   156,
       1,     2,     3,    49,  -106,   175,    45,   180,    88,     9,
     178,    24,    51,    52,    53,    54,    12,    13,   101,     1,
       2,     3,   157,    71,    73,    89,   176,    88,     9,    14,
     152,   179,    92,   117,    46,    12,    13,   121,   173,    42,
      59,    24,    57,    43,   115,    94,   109,   110,    14,   181,
      93,   118,    69,    70,   122,   103,    60,   130,   131,   132,
     133,    76,    76,    76,    76,    58,    95,    96,    97,   119,
     120,   121,    98,    99,    24,    88,    61,   129,   105,   106,
     107,   108,   127,     6,     7,     8,   134,   135,   140,     1,
       2,     3,    89,    64,    65,    24,   139,    74,     9,   145,
     159,    82,   143,    84,    85,    12,    13,     1,     2,     3,
     102,   149,    66,    67,    68,   104,     9,   111,    14,   113,
     112,   158,   123,    12,    13,   153,   124,   126,   128,    24,
     142,   160,   148,   145,   151,   155,    14,   162,   171,    24,
     164,   167,    10,    43,   153,    83,   177,   136,   172,   160,
     137,    81,   116,   141,   125,   166,     0,   153,     0,     0,
     160,     1,     2,     3,     4,     0,     5,     6,     7,     8,
       9,     0,    10,   114,     0,     0,    11,    12,    13,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      14,     0,    15,    16,     0,    17,    18,     1,     2,     3,
       4,     0,     5,     6,     7,     8,     9,     0,    10,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,    15,    16,
       0,    17,    18
};

static const yytype_int16 yycheck[] =
{
       9,     0,    16,    20,   163,    13,    15,    45,    46,    13,
       3,     4,     5,    15,    12,    15,    12,   176,    16,    12,
      15,    20,    12,    13,    14,    18,    19,    20,    36,     3,
       4,     5,    36,    42,    43,    33,    36,    16,    12,    32,
      14,    36,    18,    18,    12,    19,    20,    16,   168,    12,
       0,    50,    18,    16,    33,    64,    28,    29,    32,   179,
      36,    36,    19,    20,    33,    74,    83,   105,   106,   107,
     108,   109,   110,   111,   112,    18,    66,    67,    68,    88,
      89,    16,    69,    70,    83,    16,     3,   104,    24,    25,
      26,    27,   101,     9,    10,    11,   109,   110,    33,     3,
       4,     5,    33,    33,    18,   104,   115,    16,    12,   123,
      14,    13,   121,    18,     3,    19,    20,     3,     4,     5,
      17,   138,    21,    22,    23,    13,    12,    30,    32,    13,
      31,   148,    12,    19,    20,   144,     3,    18,    17,   138,
      17,   150,     7,   157,    17,     3,    32,    15,    17,   148,
      16,    15,    14,    16,   163,    50,   171,   111,   165,   168,
     112,    46,    85,   118,    93,   157,    -1,   176,    -1,    -1,
     179,     3,     4,     5,     6,    -1,     8,     9,    10,    11,
      12,    -1,    14,    15,    -1,    -1,    18,    19,    20,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      32,    -1,    34,    35,    -1,    37,    38,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    -1,    14,    -1,
      -1,    -1,    18,    19,    20,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    34,    35,
      -1,    37,    38
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     8,     9,    10,    11,    12,
      14,    18,    19,    20,    32,    34,    35,    37,    38,    41,
      42,    43,    44,    45,    46,    48,    49,    51,    52,    53,
      55,    56,    57,    58,    60,    62,    63,    64,    69,    70,
      71,    87,    12,    16,    47,    12,    12,    46,    58,    15,
      50,    62,    62,    62,    18,    58,    44,    18,    18,     0,
      43,     3,    72,    75,    33,    18,    21,    22,    23,    19,
      20,    58,    61,    58,    16,    59,    64,    65,    66,    67,
      68,    59,    13,    42,    18,     3,    73,    77,    16,    33,
      74,    88,    18,    36,    58,    62,    62,    62,    63,    63,
      13,    36,    17,    58,    13,    24,    25,    26,    27,    28,
      29,    30,    31,    13,    15,    33,    74,    18,    36,    58,
      58,    16,    33,    12,     3,    75,    18,    58,    17,    43,
      64,    64,    64,    64,    65,    65,    66,    67,    54,    58,
      33,    77,    17,    58,    76,    44,    85,    86,     7,    43,
      78,    17,    14,    58,    79,     3,    13,    36,    43,    14,
      58,    82,    15,    80,    16,    89,    85,    15,    83,    79,
      81,    17,    49,    82,    84,    15,    36,    47,    15,    36,
      79,    82
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    40,    41,    42,    42,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    44,    44,    44,    45,
      46,    46,    47,    47,    48,    50,    49,    49,    51,    52,
      52,    54,    53,    55,    56,    57,    57,    58,    59,    60,
      60,    60,    60,    61,    61,    61,    62,    62,    62,    62,
      62,    63,    63,    63,    63,    64,    64,    64,    65,    65,
      65,    65,    65,    66,    66,    66,    67,    67,    68,    68,
      69,    69,    70,    71,    72,    72,    73,    73,    74,    74,
      75,    75,    75,    76,    75,    77,    78,    77,    79,    80,
      79,    79,    81,    81,    82,    82,    83,    82,    84,    84,
      85,    85,    85,    86,    86,    86,    88,    89,    87
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     2,     3,     4,     4,     0,     4,     2,     1,     5,
       7,     0,     6,     2,     2,     3,     2,     1,     1,     1,
       1,     1,     3,     1,     3,     0,     1,     2,     2,     2,
       4,     1,     3,     3,     3,     1,     3,     3,     1,     3,
       3,     3,     3,     1,     3,     3,     1,     3,     1,     3,
       1,     1,     3,     4,     1,     3,     1,     3,     3,     4,
       1,     3,     2,     0,     5,     3,     0,     5,     1,     0,
       4,     2,     1,     3,     1,     2,     0,     4,     1,     3,
       2,     4,     5,     1,     3,     0,     0,     0,     8
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* Program: Stmts  */
#line 74 "src/parser.y"
            {
        ast.setRoot((yyvsp[0].stmttype));
    }
#line 1315 "src/parser.cpp"
    break;

  case 3: /* Stmts: Stmt  */
#line 79 "src/parser.y"
           {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1321 "src/parser.cpp"
    break;

  case 4: /* Stmts: Stmts Stmt  */
#line 80 "src/parser.y"
                {
        (yyval.stmttype) = new SeqNode((yyvsp[-1].stmttype), (yyvsp[0].stmttype));
    }
#line 1329 "src/parser.cpp"
    break;

  case 5: /* Stmt: AssignStmt  */
#line 85 "src/parser.y"
                 {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1335 "src/parser.cpp"
    break;

  case 6: /* Stmt: BlockStmt  */
#line 86 "src/parser.y"
                {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1341 "src/parser.cpp"
    break;

  case 7: /* Stmt: IfStmt  */
#line 87 "src/parser.y"
             {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1347 "src/parser.cpp"
    break;

  case 8: /* Stmt: WhileStmt  */
#line 88 "src/parser.y"
                {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1353 "src/parser.cpp"
    break;

  case 9: /* Stmt: ReturnStmt  */
#line 89 "src/parser.y"
                 {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1359 "src/parser.cpp"
    break;

  case 10: /* Stmt: DeclStmt  */
#line 90 "src/parser.y"
               {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1365 "src/parser.cpp"
    break;

  case 11: /* Stmt: FuncDef  */
#line 91 "src/parser.y"
              {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1371 "src/parser.cpp"
    break;

  case 12: /* Stmt: ExprStmt  */
#line 92 "src/parser.y"
               {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1377 "src/parser.cpp"
    break;

  case 13: /* Stmt: NullStmt  */
#line 93 "src/parser.y"
               {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1383 "src/parser.cpp"
    break;

  case 14: /* Stmt: ContinueStmt  */
#line 94 "src/parser.y"
                   {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1389 "src/parser.cpp"
    break;

  case 15: /* Stmt: BreakStmt  */
#line 95 "src/parser.y"
                {(yyval.stmttype)=(yyvsp[0].stmttype);}
#line 1395 "src/parser.cpp"
    break;

  case 16: /* Type: INT  */
#line 98 "src/parser.y"
          {
        (yyval.type) = curType = TypeSystem::intType;
    }
#line 1403 "src/parser.cpp"
    break;

  case 17: /* Type: FLOAT  */
#line 101 "src/parser.y"
            {
        (yyval.type) = curType = TypeSystem::floatType;
    }
#line 1411 "src/parser.cpp"
    break;

  case 18: /* Type: VOID  */
#line 104 "src/parser.y"
           {
        (yyval.type) = curType = TypeSystem::voidType;
    }
#line 1419 "src/parser.cpp"
    break;

  case 19: /* ExprStmt: Exp SEMICOLON  */
#line 111 "src/parser.y"
                  {
        (yyval.stmttype) = new ExprStmt((yyvsp[-1].exprtype));
    }
#line 1427 "src/parser.cpp"
    break;

  case 20: /* LVal: ID  */
#line 116 "src/parser.y"
         {
        SymbolEntry *se = identifiers->lookup((yyvsp[0].strtype));

        //变量（常量）使用时未声明（类型检查基础要求1）
        if(se == nullptr)
        {
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)(yyvsp[0].strtype));
            delete [](char*)(yyvsp[0].strtype);
            assert(se != nullptr);
        }

        (yyval.exprtype) = new Id(se);
        delete [](yyvsp[0].strtype);
    }
#line 1446 "src/parser.cpp"
    break;

  case 21: /* LVal: ID ArrayIndices  */
#line 130 "src/parser.y"
                     {  //数组
        SymbolEntry *se = identifiers->lookup((yyvsp[-1].strtype));

        //变量（常量）使用时未声明（类型检查基础要求1）
        if(se == nullptr)
        {
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)(yyvsp[-1].strtype));
            delete [](char*)(yyvsp[-1].strtype);
            assert(se != nullptr);
        }

        (yyval.exprtype) = new Id(se, (yyvsp[0].exprtype));
        delete [](yyvsp[-1].strtype);
    }
#line 1465 "src/parser.cpp"
    break;

  case 22: /* ArrayIndices: LBRACKET Exp RBRACKET  */
#line 147 "src/parser.y"
                            {
        (yyval.exprtype) = (yyvsp[-1].exprtype);
    }
#line 1473 "src/parser.cpp"
    break;

  case 23: /* ArrayIndices: ArrayIndices LBRACKET Exp RBRACKET  */
#line 150 "src/parser.y"
                                         {
        (yyval.exprtype) = (yyvsp[-3].exprtype);
        (yyval.exprtype)->setNext((yyvsp[-1].exprtype));
    }
#line 1482 "src/parser.cpp"
    break;

  case 24: /* AssignStmt: LVal ASSIGN Exp SEMICOLON  */
#line 158 "src/parser.y"
                              {
        (yyval.stmttype) = new AssignStmt((yyvsp[-3].exprtype), (yyvsp[-1].exprtype));
    }
#line 1490 "src/parser.cpp"
    break;

  case 25: /* $@1: %empty  */
#line 164 "src/parser.y"
        {   
            identifiers = new SymbolTable(identifiers);
        }
#line 1498 "src/parser.cpp"
    break;

  case 26: /* BlockStmt: LBRACE $@1 Stmts RBRACE  */
#line 168 "src/parser.y"
        {
            (yyval.stmttype) = new CompoundStmt((yyvsp[-1].stmttype));
            SymbolTable *top = identifiers;
            identifiers = identifiers->getPrev();
            delete top;
        }
#line 1509 "src/parser.cpp"
    break;

  case 27: /* BlockStmt: LBRACE RBRACE  */
#line 175 "src/parser.y"
                  {
        (yyval.stmttype) = new CompoundStmt(new NullStmt());
    }
#line 1517 "src/parser.cpp"
    break;

  case 28: /* NullStmt: SEMICOLON  */
#line 181 "src/parser.y"
              {
        (yyval.stmttype) = new NullStmt();
    }
#line 1525 "src/parser.cpp"
    break;

  case 29: /* IfStmt: IF LPAREN Cond RPAREN Stmt  */
#line 188 "src/parser.y"
                                            {
        (yyval.stmttype) = new IfStmt((yyvsp[-2].exprtype), (yyvsp[0].stmttype));
    }
#line 1533 "src/parser.cpp"
    break;

  case 30: /* IfStmt: IF LPAREN Cond RPAREN Stmt ELSE Stmt  */
#line 191 "src/parser.y"
                                           {
        (yyval.stmttype) = new IfElseStmt((yyvsp[-4].exprtype), (yyvsp[-2].stmttype), (yyvsp[0].stmttype));
    }
#line 1541 "src/parser.cpp"
    break;

  case 31: /* $@2: %empty  */
#line 197 "src/parser.y"
                            {
        useforwhilecheck++;
    }
#line 1549 "src/parser.cpp"
    break;

  case 32: /* WhileStmt: WHILE LPAREN Cond RPAREN $@2 Stmt  */
#line 199 "src/parser.y"
           {
        (yyval.stmttype) = new WhileStmt((yyvsp[-3].exprtype),(yyvsp[0].stmttype));
        useforwhilecheck--;
    }
#line 1558 "src/parser.cpp"
    break;

  case 33: /* BreakStmt: BREAK SEMICOLON  */
#line 206 "src/parser.y"
                    {
        //静态检查：检查break是否在while中（类型检查基础要求6）
        if (!useforwhilecheck) {
            fprintf(stderr, "\"break\" not in whilestmt\n");
            assert(useforwhilecheck);        
        }
        (yyval.stmttype) = new BreakStmt();
    }
#line 1571 "src/parser.cpp"
    break;

  case 34: /* ContinueStmt: CONTINUE SEMICOLON  */
#line 217 "src/parser.y"
                       {
        //静态检查：检查continue是否在while中（类型检查基础要求6）
        if (!useforwhilecheck) {
            fprintf(stderr, "\"continue\" not in whilestmt\n");
            assert(useforwhilecheck);        
        }
        (yyval.stmttype) = new ContinueStmt();
    }
#line 1584 "src/parser.cpp"
    break;

  case 35: /* ReturnStmt: RETURN Exp SEMICOLON  */
#line 228 "src/parser.y"
                        {
        //返回值的类型检查会主体在ReturnStmt::checkRet中实现（类型检查基础要求5）
        (yyval.stmttype) = new ReturnStmt((yyvsp[-1].exprtype));
    }
#line 1593 "src/parser.cpp"
    break;

  case 36: /* ReturnStmt: RETURN SEMICOLON  */
#line 233 "src/parser.y"
                     {
        (yyval.stmttype) = new ReturnStmt(nullptr);
    }
#line 1601 "src/parser.cpp"
    break;

  case 37: /* Exp: AddExp  */
#line 242 "src/parser.y"
           {(yyval.exprtype) = (yyvsp[0].exprtype);}
#line 1607 "src/parser.cpp"
    break;

  case 38: /* Cond: LOrExp  */
#line 247 "src/parser.y"
           {(yyval.exprtype) = (yyvsp[0].exprtype);}
#line 1613 "src/parser.cpp"
    break;

  case 39: /* PrimaryExp: LVal  */
#line 252 "src/parser.y"
         {
        (yyval.exprtype) = (yyvsp[0].exprtype);
    }
#line 1621 "src/parser.cpp"
    break;

  case 40: /* PrimaryExp: INTEGER  */
#line 255 "src/parser.y"
              {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::intType, (yyvsp[0].itype));
        (yyval.exprtype) = new Constant(se);
    }
#line 1630 "src/parser.cpp"
    break;

  case 41: /* PrimaryExp: FLOATNUMBER  */
#line 259 "src/parser.y"
                  {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::floatType, (yyvsp[0].ftype));
        (yyval.exprtype) = new Constant(se);
    }
#line 1639 "src/parser.cpp"
    break;

  case 42: /* PrimaryExp: LPAREN Exp RPAREN  */
#line 264 "src/parser.y"
                      {
        (yyval.exprtype) = (yyvsp[-1].exprtype);
    }
#line 1647 "src/parser.cpp"
    break;

  case 43: /* FuncRParams: Exp  */
#line 269 "src/parser.y"
          {
        (yyval.exprtype) = (yyvsp[0].exprtype);
    }
#line 1655 "src/parser.cpp"
    break;

  case 44: /* FuncRParams: FuncRParams COMMA Exp  */
#line 272 "src/parser.y"
                            {
        (yyval.exprtype) = (yyvsp[-2].exprtype);
        (yyval.exprtype)->setNext((yyvsp[0].exprtype));
    }
#line 1664 "src/parser.cpp"
    break;

  case 45: /* FuncRParams: %empty  */
#line 276 "src/parser.y"
             {
       (yyval.exprtype) = nullptr;
    }
#line 1672 "src/parser.cpp"
    break;

  case 46: /* UnaryExp: PrimaryExp  */
#line 283 "src/parser.y"
              {(yyval.exprtype)=(yyvsp[0].exprtype);}
#line 1678 "src/parser.cpp"
    break;

  case 47: /* UnaryExp: ADD UnaryExp  */
#line 285 "src/parser.y"
                 {(yyval.exprtype) = (yyvsp[0].exprtype);}
#line 1684 "src/parser.cpp"
    break;

  case 48: /* UnaryExp: SUB UnaryExp  */
#line 287 "src/parser.y"
                 {
        SymbolEntry *se = new TemporarySymbolEntry((yyvsp[0].exprtype)->getType(), SymbolTable::getLabel());
        (yyval.exprtype) = new UnaryExpr(se,UnaryExpr::MINUS,(yyvsp[0].exprtype));
    }
#line 1693 "src/parser.cpp"
    break;

  case 49: /* UnaryExp: NOT UnaryExp  */
#line 292 "src/parser.y"
                 {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new UnaryExpr(se,UnaryExpr::NOT,(yyvsp[0].exprtype));
    }
#line 1702 "src/parser.cpp"
    break;

  case 50: /* UnaryExp: ID LPAREN FuncRParams RPAREN  */
#line 297 "src/parser.y"
                                 {
        SymbolEntry *se = identifiers->lookup((yyvsp[-3].strtype));
        SymbolEntry *tmp = new TemporarySymbolEntry(dynamic_cast<FunctionType*>(se->getType())->getRetType(), SymbolTable::getLabel());
        //检查未声明函数（类型检查基础要求4）
        if(se == nullptr){
            fprintf(stderr, "function identifier \"%s\" is undefined\n", (char*)(yyvsp[-3].strtype));
            delete [](char*)(yyvsp[-3].strtype);
            assert(se != nullptr);        
        }
        //如果有参数，FuncCall函数中有检查函数形参是否与实参类型及数目匹配的逻辑（类型检查基础要求4）
        else if((yyvsp[-1].exprtype) != nullptr){
            (yyval.exprtype) = new FuncCall(tmp,se, ((ExprNode*)(yyvsp[-1].exprtype)));        
        }
        //如果没有参数
        else {
            (yyval.exprtype) = new FuncCall(tmp,se, nullptr);
        }
    }
#line 1725 "src/parser.cpp"
    break;

  case 51: /* MULExp: UnaryExp  */
#line 319 "src/parser.y"
             {(yyval.exprtype) = (yyvsp[0].exprtype);}
#line 1731 "src/parser.cpp"
    break;

  case 52: /* MULExp: MULExp MUL UnaryExp  */
#line 321 "src/parser.y"
                        {
        SymbolEntry *se ;
        if ((yyvsp[-2].exprtype)->getType()->isFloat() || (yyvsp[0].exprtype)->getType()->isFloat()){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else{
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        }
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::MUL, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1746 "src/parser.cpp"
    break;

  case 53: /* MULExp: MULExp DIV UnaryExp  */
#line 332 "src/parser.y"
                        {
        SymbolEntry *se ;
        if ((yyvsp[-2].exprtype)->getType()->isFloat() || (yyvsp[0].exprtype)->getType()->isFloat()){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else{
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        }        
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::DIV, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1761 "src/parser.cpp"
    break;

  case 54: /* MULExp: MULExp MOD UnaryExp  */
#line 343 "src/parser.y"
                        {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::MOD, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1770 "src/parser.cpp"
    break;

  case 55: /* AddExp: MULExp  */
#line 351 "src/parser.y"
           {(yyval.exprtype) = (yyvsp[0].exprtype);}
#line 1776 "src/parser.cpp"
    break;

  case 56: /* AddExp: AddExp ADD MULExp  */
#line 354 "src/parser.y"
    {
        SymbolEntry *se ;
        if ((yyvsp[-2].exprtype)->getType()->isFloat() || (yyvsp[0].exprtype)->getType()->isFloat()){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else{
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        }
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::ADD, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1791 "src/parser.cpp"
    break;

  case 57: /* AddExp: AddExp SUB MULExp  */
#line 366 "src/parser.y"
    {
        SymbolEntry *se ;
        if ((yyvsp[-2].exprtype)->getType()->isFloat() || (yyvsp[0].exprtype)->getType()->isFloat()){
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        }
        else{
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        }    
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::SUB, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1806 "src/parser.cpp"
    break;

  case 58: /* RelExp: AddExp  */
#line 380 "src/parser.y"
           {(yyval.exprtype) = (yyvsp[0].exprtype);}
#line 1812 "src/parser.cpp"
    break;

  case 59: /* RelExp: RelExp LESS AddExp  */
#line 383 "src/parser.y"
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::LESS, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1821 "src/parser.cpp"
    break;

  case 60: /* RelExp: RelExp LESSEQL AddExp  */
#line 388 "src/parser.y"
                          {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::LESSEQL, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1830 "src/parser.cpp"
    break;

  case 61: /* RelExp: RelExp GREATER AddExp  */
#line 393 "src/parser.y"
                          {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::GREATER, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1839 "src/parser.cpp"
    break;

  case 62: /* RelExp: RelExp GREATEREQL AddExp  */
#line 398 "src/parser.y"
                             {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::GREATEREQL, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1848 "src/parser.cpp"
    break;

  case 63: /* EqlExp: RelExp  */
#line 406 "src/parser.y"
           {(yyval.exprtype)=(yyvsp[0].exprtype);}
#line 1854 "src/parser.cpp"
    break;

  case 64: /* EqlExp: EqlExp EQL RelExp  */
#line 408 "src/parser.y"
                      {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::EQL, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1863 "src/parser.cpp"
    break;

  case 65: /* EqlExp: EqlExp NOTEQL RelExp  */
#line 413 "src/parser.y"
                         {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::NOTEQL, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1872 "src/parser.cpp"
    break;

  case 66: /* LAndExp: EqlExp  */
#line 421 "src/parser.y"
           {(yyval.exprtype) = (yyvsp[0].exprtype);}
#line 1878 "src/parser.cpp"
    break;

  case 67: /* LAndExp: LAndExp AND EqlExp  */
#line 424 "src/parser.y"
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::AND, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1887 "src/parser.cpp"
    break;

  case 68: /* LOrExp: LAndExp  */
#line 432 "src/parser.y"
            {(yyval.exprtype) = (yyvsp[0].exprtype);}
#line 1893 "src/parser.cpp"
    break;

  case 69: /* LOrExp: LOrExp OR LAndExp  */
#line 435 "src/parser.y"
    {
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        (yyval.exprtype) = new BinaryExpr(se, BinaryExpr::OR, (yyvsp[-2].exprtype), (yyvsp[0].exprtype));
    }
#line 1902 "src/parser.cpp"
    break;

  case 70: /* DeclStmt: VarDeclStmt  */
#line 445 "src/parser.y"
                  {(yyval.stmttype) = (yyvsp[0].stmttype);}
#line 1908 "src/parser.cpp"
    break;

  case 71: /* DeclStmt: ConstDeclStmt  */
#line 446 "src/parser.y"
                    {(yyval.stmttype) = (yyvsp[0].stmttype);}
#line 1914 "src/parser.cpp"
    break;

  case 72: /* VarDeclStmt: Type VarDefList SEMICOLON  */
#line 450 "src/parser.y"
                              {
        //变量的类型不能为void（类型检查基础要求1）
        if((yyvsp[-2].type)==TypeSystem::voidType)
        {
            fprintf(stderr, "variable type can't be void type\n");   
            assert((yyvsp[-2].type)!=TypeSystem::voidType);
        }
        (yyval.stmttype) = (yyvsp[-1].stmttype);
    }
#line 1928 "src/parser.cpp"
    break;

  case 73: /* ConstDeclStmt: CONST Type ConstDefList SEMICOLON  */
#line 463 "src/parser.y"
                                      {
        //常量的类型不能为void（类型检查基础要求1）
        if((yyvsp[-2].type)==TypeSystem::voidType)
        {
            fprintf(stderr, "constant variable type can't be void type\n");   
            assert((yyvsp[-2].type)!=TypeSystem::voidType);
        }
        (yyval.stmttype) = (yyvsp[-1].stmttype);
    }
#line 1942 "src/parser.cpp"
    break;

  case 74: /* VarDefList: VarDef  */
#line 476 "src/parser.y"
           {
        (yyval.stmttype) = (yyvsp[0].stmttype);
    }
#line 1950 "src/parser.cpp"
    break;

  case 75: /* VarDefList: VarDefList COMMA VarDef  */
#line 480 "src/parser.y"
                            {
        (yyval.stmttype) = (yyvsp[-2].stmttype);
        (yyvsp[-2].stmttype)->setNext((yyvsp[0].stmttype)); //参数表的参数通过指针串起来
    }
#line 1959 "src/parser.cpp"
    break;

  case 76: /* ConstDefList: ConstDef  */
#line 487 "src/parser.y"
             {
        (yyval.stmttype) = (yyvsp[0].stmttype); 
    }
#line 1967 "src/parser.cpp"
    break;

  case 77: /* ConstDefList: ConstDefList COMMA ConstDef  */
#line 491 "src/parser.y"
                                {
        (yyval.stmttype) = (yyvsp[-2].stmttype);
        (yyvsp[-2].stmttype)->setNext((yyvsp[0].stmttype));
    }
#line 1976 "src/parser.cpp"
    break;

  case 78: /* DeclArrayIndices: LBRACKET Exp RBRACKET  */
#line 497 "src/parser.y"
                            {
        (yyval.exprtype) = (yyvsp[-1].exprtype);
    }
#line 1984 "src/parser.cpp"
    break;

  case 79: /* DeclArrayIndices: DeclArrayIndices LBRACKET Exp RBRACKET  */
#line 500 "src/parser.y"
                                             {
        (yyval.exprtype) = (yyvsp[-3].exprtype);
        (yyval.exprtype)->setNext((yyvsp[-1].exprtype));
    }
#line 1993 "src/parser.cpp"
    break;

  case 80: /* VarDef: ID  */
#line 507 "src/parser.y"
       {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent((yyvsp[0].strtype)))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)(yyvsp[0].strtype));
            assert(identifiers->lookuponlyforcurrent((yyvsp[0].strtype))==nullptr);
        }
        SymbolEntry* se = new IdentifierSymbolEntry(curType, (yyvsp[0].strtype), identifiers->getLevel());
        (yyval.stmttype) = new DeclStmt(new Id(se));
        identifiers->install((yyvsp[0].strtype), se);
        delete [](yyvsp[0].strtype);
    }
#line 2010 "src/parser.cpp"
    break;

  case 81: /* VarDef: ID ASSIGN Exp  */
#line 520 "src/parser.y"
                  {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent((yyvsp[-2].strtype)))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)(yyvsp[-2].strtype));
            assert(identifiers->lookuponlyforcurrent((yyvsp[-2].strtype))==nullptr);
        }
        SymbolEntry* se = new IdentifierSymbolEntry(curType, (yyvsp[-2].strtype), identifiers->getLevel());
        (yyval.stmttype) = new DeclStmt(new Id(se), (yyvsp[0].exprtype));
        identifiers->install((yyvsp[-2].strtype), se);
        ((IdentifierSymbolEntry*)se)->setValue((yyvsp[0].exprtype)->getValue());
        delete [](yyvsp[-2].strtype);
    }
#line 2028 "src/parser.cpp"
    break;

  case 82: /* VarDef: ID DeclArrayIndices  */
#line 533 "src/parser.y"
                        {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent((yyvsp[-1].strtype)))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)(yyvsp[-1].strtype));
            assert(identifiers->lookuponlyforcurrent((yyvsp[-1].strtype))==nullptr);
        }
        
        ExprNode *expr = (yyvsp[0].exprtype);
        std::vector<int> indexs;
        while(expr) {
            indexs.push_back(expr->getValue());
            expr = (ExprNode*)expr->getNext();
        }
        Type* arrType = new ArrayType(indexs, curType);

        SymbolEntry* se = new IdentifierSymbolEntry(arrType, (yyvsp[-1].strtype), identifiers->getLevel());
        identifiers->install((yyvsp[-1].strtype), se);
        (yyval.stmttype) = new DeclStmt(new Id(se));
        delete [](yyvsp[-1].strtype);
    }
#line 2054 "src/parser.cpp"
    break;

  case 83: /* @3: %empty  */
#line 554 "src/parser.y"
                                {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent((yyvsp[-2].strtype)))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)(yyvsp[-2].strtype));
            assert(identifiers->lookuponlyforcurrent((yyvsp[-2].strtype))==nullptr);
        }
        
        ExprNode *expr = (yyvsp[-1].exprtype);
        std::vector<int> indexs;
        if(expr== nullptr) printf("this is a null\n");
        while(expr) {
            indexs.push_back(expr->getValue());
            expr = (ExprNode*)expr->getNext();
        }
        auto arrType = new ArrayType(indexs, curType);

        SymbolEntry* se = new IdentifierSymbolEntry(arrType, (yyvsp[-2].strtype), identifiers->getLevel());
        (yyval.se) = se;
        identifiers->install((yyvsp[-2].strtype), se);

        initArray = new ExprNode*[arrType->getSize() / arrType->getBaseType()->getSize()] {};
        idx = 0;
        std::stack<std::vector<int>>().swap(dimesionStack); // 这一句的作用就是清空栈
        dimesionStack.push(indexs);
        delete [](yyvsp[-2].strtype);

    }
#line 2087 "src/parser.cpp"
    break;

  case 84: /* VarDef: ID DeclArrayIndices ASSIGN @3 InitVal  */
#line 581 "src/parser.y"
              {
        (yyval.stmttype) = new DeclStmt(new Id((yyvsp[-1].se), (yyvsp[-3].exprtype)));
        ((DeclStmt*)(yyval.stmttype))->setInitArray(initArray);
        initArray = nullptr;
        idx = 0;
    }
#line 2098 "src/parser.cpp"
    break;

  case 85: /* ConstDef: ID ASSIGN Exp  */
#line 592 "src/parser.y"
                  {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent((yyvsp[-2].strtype)))
        {
            fprintf(stderr, "constant identifier \"%s\" is defined twice\n", (char*)(yyvsp[-2].strtype));
            assert(identifiers->lookuponlyforcurrent((yyvsp[-2].strtype))==nullptr);
        }
        SymbolEntry* se = nullptr;
        if (curType->isInt()) {
            se = new IdentifierSymbolEntry(TypeSystem::constIntType, (yyvsp[-2].strtype), identifiers->getLevel());
            ((IdentifierSymbolEntry*)se)->setValue((int)((yyvsp[0].exprtype)->getValue()));
        }
        else if (curType->isFloat()) {
            se = new IdentifierSymbolEntry(TypeSystem::constFloatType, (yyvsp[-2].strtype), identifiers->getLevel());
            ((IdentifierSymbolEntry*)se)->setValue((double)((yyvsp[0].exprtype)->getValue()));
        }
        identifiers->install((yyvsp[-2].strtype), se);
        ((IdentifierSymbolEntry*)se)->setConstant();
        (yyval.stmttype) = new DeclStmt(new Id(se), (yyvsp[0].exprtype));
        delete [](yyvsp[-2].strtype);
    }
#line 2124 "src/parser.cpp"
    break;

  case 86: /* @4: %empty  */
#line 613 "src/parser.y"
                                {
        //检查变量是否重复声明（类型检查基础要求1）
        if(identifiers->lookuponlyforcurrent((yyvsp[-2].strtype)))
        {
            fprintf(stderr, "identifier \"%s\" is defined twice\n", (char*)(yyvsp[-2].strtype));
            assert(identifiers->lookuponlyforcurrent((yyvsp[-2].strtype))==nullptr);
        }
        

        ExprNode* expr = (yyvsp[-1].exprtype);
        std::vector<int> indexs;
        while(expr) {
            indexs.push_back(expr->getValue());
            expr = (ExprNode*)expr->getNext();
        }
        ArrayType* arrType = nullptr;
        if(curType->isInt()) {
            arrType = new ArrayType(indexs, TypeSystem::constIntType);
        }
        else if(curType->isFloat()) {
            arrType = new ArrayType(indexs, TypeSystem::constFloatType);
        }
        assert(arrType != nullptr);
        SymbolEntry* se = new IdentifierSymbolEntry(arrType, (yyvsp[-2].strtype), identifiers->getLevel());
        ((IdentifierSymbolEntry*)se)->setConstant();
        (yyval.se) = se;
        identifiers->install((yyvsp[-2].strtype), se);

        initArray = new ExprNode*[arrType->getSize() / arrType->getBaseType()->getSize()] {};
        idx = 0;
        std::stack<std::vector<int>>().swap(dimesionStack); // 这一句的作用就是清空栈
        dimesionStack.push(indexs);

        delete [](yyvsp[-2].strtype);
    }
#line 2164 "src/parser.cpp"
    break;

  case 87: /* ConstDef: ID DeclArrayIndices ASSIGN @4 ConstInitVal  */
#line 647 "src/parser.y"
                   {
        (yyval.stmttype) = new DeclStmt(new Id((yyvsp[-1].se)));
        ((DeclStmt*)(yyval.stmttype))->setInitArray(initArray);
        initArray = nullptr;
        idx = 0;
    }
#line 2175 "src/parser.cpp"
    break;

  case 88: /* InitVal: Exp  */
#line 656 "src/parser.y"
        {
        (yyval.exprtype) = (yyvsp[0].exprtype);
        if(initArray != nullptr) {
            initArray[idx++] = (yyvsp[0].exprtype);
        }
    }
#line 2186 "src/parser.cpp"
    break;

  case 89: /* $@5: %empty  */
#line 663 "src/parser.y"
           {
        std::vector<int> dimesion = dimesionStack.top();
        dimesionStack.push({-1, idx});
        dimesion.erase(dimesion.begin());
        if (dimesion.size() <= 0) {
            dimesion.push_back(1);
        }
        dimesionStack.push(dimesion);
    }
#line 2200 "src/parser.cpp"
    break;

  case 90: /* InitVal: LBRACE $@5 InitValList RBRACE  */
#line 671 "src/parser.y"
                         {
        while (dimesionStack.top()[0] != -1) {
            dimesionStack.pop();
        }
        idx = dimesionStack.top()[1];
        dimesionStack.pop();
        std::vector<int> dimesion = dimesionStack.top();
        int size = 1;
        for (auto dim : dimesion) {
            size *= dim;
        }
        idx += size;
    }
#line 2218 "src/parser.cpp"
    break;

  case 91: /* InitVal: LBRACE RBRACE  */
#line 685 "src/parser.y"
                  {
        std::vector<int> dimesion = dimesionStack.top();
        int size = 1;
        for(auto dim : dimesion) {
            size *= dim;
        }
        idx += size;
    }
#line 2231 "src/parser.cpp"
    break;

  case 94: /* ConstInitVal: Exp  */
#line 702 "src/parser.y"
          {
        (yyval.exprtype) = (yyvsp[0].exprtype);
        if (initArray != nullptr) {
            initArray[idx++] = (yyvsp[0].exprtype);
        }
    }
#line 2242 "src/parser.cpp"
    break;

  case 95: /* ConstInitVal: LBRACE RBRACE  */
#line 708 "src/parser.y"
                    {
        std::vector<int> dimesion = dimesionStack.top();
        int size = 1;
        for (auto dim : dimesion) {
            size *= dim;
        }
        idx += size;
    }
#line 2255 "src/parser.cpp"
    break;

  case 96: /* $@6: %empty  */
#line 716 "src/parser.y"
             {
        std::vector<int> dimesion = dimesionStack.top();
        dimesionStack.push({-1, idx});
        dimesion.erase(dimesion.begin());
        if (dimesion.size() <= 0) {
            dimesion.push_back(1);
        }
        dimesionStack.push(dimesion);
    }
#line 2269 "src/parser.cpp"
    break;

  case 97: /* ConstInitVal: LBRACE $@6 ConstInitValList RBRACE  */
#line 724 "src/parser.y"
                              {
        while (dimesionStack.top()[0] != -1) {
            dimesionStack.pop();
        }
        idx = dimesionStack.top()[1];
        dimesionStack.pop();
        std::vector<int> dimesion = dimesionStack.top();
        int size = 1;
        for (auto dim : dimesion) {
            size *= dim;
        }
        idx += size;
    }
#line 2287 "src/parser.cpp"
    break;

  case 100: /* FuncFParam: Type ID  */
#line 748 "src/parser.y"
              {
        SymbolEntry* se;
        int argNum;
        if ((yyvsp[-1].type)->isFloat()) {
            argNum = floatArgNum;
            if (argNum > 15) {
                argNum = spillPos;
                spillPos--;
            }
            floatArgNum++;
        }
        else {
            argNum = intArgNum;
            if (argNum > 3) {
                argNum = spillPos;
                spillPos--;
            }
            intArgNum++;
        }
        se = new IdentifierSymbolEntry((yyvsp[-1].type), (yyvsp[0].strtype), identifiers->getLevel(), false, argNum);
        identifiers->install((yyvsp[0].strtype), se);
        (yyval.stmttype) = new DeclStmt(new Id(se));
        delete [](yyvsp[0].strtype);
    }
#line 2316 "src/parser.cpp"
    break;

  case 101: /* FuncFParam: Type ID LBRACKET RBRACKET  */
#line 772 "src/parser.y"
                                {
        SymbolEntry* se;
        int argNum = intArgNum;
        if (argNum > 3) {
            argNum = spillPos;
            spillPos--;
        }
        intArgNum++;
        se = new IdentifierSymbolEntry(new PointerType(new ArrayType({}, (yyvsp[-3].type))), (yyvsp[-2].strtype), identifiers->getLevel(), false, argNum);
        identifiers->install((yyvsp[-2].strtype), se);
        (yyval.stmttype) = new DeclStmt(new Id(se));
        delete [](yyvsp[-2].strtype);
    }
#line 2334 "src/parser.cpp"
    break;

  case 102: /* FuncFParam: Type ID LBRACKET RBRACKET ArrayIndices  */
#line 785 "src/parser.y"
                                             {
        std::vector<int> indexs;
        ExprNode *expr = (yyvsp[0].exprtype);
        while (expr) {
            indexs.push_back(expr->getValue());
            expr = (ExprNode*)expr->getNext();
        }
        int argNum = intArgNum;
        if (argNum > 3) {
            argNum = spillPos;
            spillPos--;
        }
        intArgNum++;
        SymbolEntry* se;
        se = new IdentifierSymbolEntry(new PointerType(new ArrayType(indexs, (yyvsp[-4].type))), (yyvsp[-3].strtype), identifiers->getLevel(), false, argNum);
        identifiers->install((yyvsp[-3].strtype), se);
        (yyval.stmttype) = new DeclStmt(new Id(se));
        delete [](yyvsp[-3].strtype);
    }
#line 2358 "src/parser.cpp"
    break;

  case 103: /* FuncFParams: FuncFParam  */
#line 808 "src/parser.y"
               {
        (yyval.stmttype) = (yyvsp[0].stmttype);
    }
#line 2366 "src/parser.cpp"
    break;

  case 104: /* FuncFParams: FuncFParams COMMA FuncFParam  */
#line 812 "src/parser.y"
                                 {
        (yyval.stmttype) = (yyvsp[-2].stmttype);
        (yyval.stmttype)->setNext((yyvsp[0].stmttype));
    }
#line 2375 "src/parser.cpp"
    break;

  case 105: /* FuncFParams: %empty  */
#line 817 "src/parser.y"
           {
        (yyval.stmttype) = nullptr;
    }
#line 2383 "src/parser.cpp"
    break;

  case 106: /* $@7: %empty  */
#line 824 "src/parser.y"
            {
        //retType = $1;
        // needRet = true;
        identifiers = new SymbolTable(identifiers);
        spillPos = -1;
        intArgNum = 0;
        floatArgNum = 0;
    }
#line 2396 "src/parser.cpp"
    break;

  case 107: /* $@8: %empty  */
#line 832 "src/parser.y"
                              {
        Type* funcType;
        std::vector<Type*> vec;
        std::vector<SymbolEntry*> vec1;
        DeclStmt* temp = (DeclStmt*)(yyvsp[-1].stmttype);
        while(temp){
            vec.push_back(temp->getId()->getSymPtr()->getType());
            vec1.push_back(temp->getId()->getSymPtr());
            temp = (DeclStmt*)(temp->getNext());
        }
        funcType = new FunctionType((yyvsp[-5].type), vec, vec1);
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, (yyvsp[-4].strtype), identifiers->getPrev()->getLevel());
        //install中有检查重载函数的逻辑，在SymbolTable.cpp中查看（类型检查进阶要求1）
        identifiers->getPrev()->install((yyvsp[-4].strtype), se);
    }
#line 2416 "src/parser.cpp"
    break;

  case 108: /* FuncDef: Type ID $@7 LPAREN FuncFParams RPAREN $@8 BlockStmt  */
#line 848 "src/parser.y"
    {
        SymbolEntry *se = identifiers->lookup((yyvsp[-6].strtype));
        assert(se != nullptr);
        (yyval.stmttype) = new FunctionDef(se, (yyvsp[0].stmttype), (DeclStmt*)(yyvsp[-3].stmttype));
        SymbolTable *top = identifiers;
        identifiers = identifiers->getPrev();
        delete top;        
        delete [](yyvsp[-6].strtype);
    }
#line 2430 "src/parser.cpp"
    break;


#line 2434 "src/parser.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 866 "src/parser.y"


int yyerror(char const* message)
{
    std::cerr<<message<<std::endl;
    return -1;
}
