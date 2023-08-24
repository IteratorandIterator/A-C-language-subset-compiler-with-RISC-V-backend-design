
// Generated from /home/carbon/code/compiler_guas/grammar/SYSYParser.g4 by ANTLR 4.12.0


#include "SYSYParserListener.h"
#include "SYSYParserVisitor.h"

#include "SYSYParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct SYSYParserStaticData final {
  SYSYParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SYSYParserStaticData(const SYSYParserStaticData&) = delete;
  SYSYParserStaticData(SYSYParserStaticData&&) = delete;
  SYSYParserStaticData& operator=(const SYSYParserStaticData&) = delete;
  SYSYParserStaticData& operator=(SYSYParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag sysyparserParserOnceFlag;
SYSYParserStaticData *sysyparserParserStaticData = nullptr;

void sysyparserParserInitialize() {
  assert(sysyparserParserStaticData == nullptr);
  auto staticData = std::make_unique<SYSYParserStaticData>(
    std::vector<std::string>{
      "compilationUnit", "decl", "constdecl", "constdef", "init", "vardecl", 
      "vardef", "funcdef", "funcFparams", "funcFparam", "functype", "typeId", 
      "block", "stmt", "lval", "cond", "exp", "number"
    },
    std::vector<std::string>{
      "", "','", "';'", "'='", "'['", "']'", "'{'", "'}'", "'('", "')'", 
      "'if'", "'else'", "'while'", "'const'", "'=='", "'!='", "'<'", "'>'", 
      "'<='", "'>='", "'+'", "'-'", "'*'", "'/'", "'%'", "'!'", "'&&'", 
      "'||'", "'int'", "'float'", "'void'", "", "", "", "'break'", "'continue'", 
      "'return'"
    },
    std::vector<std::string>{
      "", "Comma", "SemiColon", "Assign", "LeftBracket", "RightBracket", 
      "LeftBrace", "RightBrace", "LeftParen", "RightParen", "If", "Else", 
      "While", "Const", "Equal", "NonEqual", "Less", "Greater", "LessEqual", 
      "GreaterEqual", "Plus", "Minus", "Multiply", "Divide", "Modulo", "Not", 
      "And", "Or", "Int", "Float", "Void", "Identifier", "FloatConst", "IntConst", 
      "Break", "Continue", "Return", "LineComment", "BlockComment", "WhiteSpace"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,39,302,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,1,0,1,0,4,0,39,8,0,11,0,12,0,40,1,1,
  	1,1,3,1,45,8,1,1,2,1,2,1,2,1,2,1,2,5,2,52,8,2,10,2,12,2,55,9,2,1,2,1,
  	2,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,4,3,67,8,3,11,3,12,3,68,1,3,1,3,1,3,
  	3,3,74,8,3,1,4,1,4,1,4,1,4,5,4,80,8,4,10,4,12,4,83,9,4,3,4,85,8,4,1,4,
  	1,4,3,4,89,8,4,1,5,1,5,1,5,1,5,5,5,95,8,5,10,5,12,5,98,9,5,1,5,1,5,1,
  	6,1,6,1,6,1,6,1,6,1,6,4,6,108,8,6,11,6,12,6,109,1,6,1,6,1,6,1,6,1,6,1,
  	6,1,6,1,6,4,6,120,8,6,11,6,12,6,121,1,6,1,6,1,6,3,6,127,8,6,1,7,1,7,1,
  	7,1,7,3,7,133,8,7,1,7,1,7,1,7,1,8,1,8,1,8,5,8,141,8,8,10,8,12,8,144,9,
  	8,1,9,1,9,1,9,1,9,1,9,1,9,1,9,1,9,5,9,154,8,9,10,9,12,9,157,9,9,3,9,159,
  	8,9,1,10,1,10,1,11,1,11,1,12,1,12,1,12,5,12,168,8,12,10,12,12,12,171,
  	9,12,1,12,1,12,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,5,13,
  	185,8,13,10,13,12,13,188,9,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,
  	1,13,1,13,1,13,1,13,1,13,1,13,1,13,3,13,205,8,13,1,13,1,13,1,13,1,13,
  	1,13,1,13,1,13,1,13,1,13,1,13,1,13,1,13,3,13,219,8,13,1,13,1,13,3,13,
  	223,8,13,1,13,3,13,226,8,13,1,14,1,14,1,14,1,14,1,14,1,14,4,14,234,8,
  	14,11,14,12,14,235,3,14,238,8,14,1,15,1,15,1,15,1,15,3,15,244,8,15,1,
  	15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,1,15,5,15,258,8,
  	15,10,15,12,15,261,9,15,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,1,16,
  	1,16,1,16,1,16,1,16,1,16,5,16,277,8,16,10,16,12,16,280,9,16,1,16,1,16,
  	1,16,1,16,1,16,3,16,287,8,16,1,16,1,16,1,16,1,16,1,16,1,16,5,16,295,8,
  	16,10,16,12,16,298,9,16,1,17,1,17,1,17,0,2,30,32,18,0,2,4,6,8,10,12,14,
  	16,18,20,22,24,26,28,30,32,34,0,8,1,0,28,29,1,0,28,30,2,0,20,21,25,25,
  	1,0,16,19,1,0,14,15,1,0,20,21,1,0,22,24,1,0,32,33,332,0,38,1,0,0,0,2,
  	44,1,0,0,0,4,46,1,0,0,0,6,73,1,0,0,0,8,88,1,0,0,0,10,90,1,0,0,0,12,126,
  	1,0,0,0,14,128,1,0,0,0,16,137,1,0,0,0,18,145,1,0,0,0,20,160,1,0,0,0,22,
  	162,1,0,0,0,24,164,1,0,0,0,26,225,1,0,0,0,28,237,1,0,0,0,30,243,1,0,0,
  	0,32,286,1,0,0,0,34,299,1,0,0,0,36,39,3,2,1,0,37,39,3,14,7,0,38,36,1,
  	0,0,0,38,37,1,0,0,0,39,40,1,0,0,0,40,38,1,0,0,0,40,41,1,0,0,0,41,1,1,
  	0,0,0,42,45,3,4,2,0,43,45,3,10,5,0,44,42,1,0,0,0,44,43,1,0,0,0,45,3,1,
  	0,0,0,46,47,5,13,0,0,47,48,7,0,0,0,48,53,3,6,3,0,49,50,5,1,0,0,50,52,
  	3,6,3,0,51,49,1,0,0,0,52,55,1,0,0,0,53,51,1,0,0,0,53,54,1,0,0,0,54,56,
  	1,0,0,0,55,53,1,0,0,0,56,57,5,2,0,0,57,5,1,0,0,0,58,59,5,31,0,0,59,60,
  	5,3,0,0,60,74,3,8,4,0,61,66,5,31,0,0,62,63,5,4,0,0,63,64,3,32,16,0,64,
  	65,5,5,0,0,65,67,1,0,0,0,66,62,1,0,0,0,67,68,1,0,0,0,68,66,1,0,0,0,68,
  	69,1,0,0,0,69,70,1,0,0,0,70,71,5,3,0,0,71,72,3,8,4,0,72,74,1,0,0,0,73,
  	58,1,0,0,0,73,61,1,0,0,0,74,7,1,0,0,0,75,84,5,6,0,0,76,81,3,8,4,0,77,
  	78,5,1,0,0,78,80,3,8,4,0,79,77,1,0,0,0,80,83,1,0,0,0,81,79,1,0,0,0,81,
  	82,1,0,0,0,82,85,1,0,0,0,83,81,1,0,0,0,84,76,1,0,0,0,84,85,1,0,0,0,85,
  	86,1,0,0,0,86,89,5,7,0,0,87,89,3,32,16,0,88,75,1,0,0,0,88,87,1,0,0,0,
  	89,9,1,0,0,0,90,91,7,0,0,0,91,96,3,12,6,0,92,93,5,1,0,0,93,95,3,12,6,
  	0,94,92,1,0,0,0,95,98,1,0,0,0,96,94,1,0,0,0,96,97,1,0,0,0,97,99,1,0,0,
  	0,98,96,1,0,0,0,99,100,5,2,0,0,100,11,1,0,0,0,101,127,5,31,0,0,102,107,
  	5,31,0,0,103,104,5,4,0,0,104,105,3,32,16,0,105,106,5,5,0,0,106,108,1,
  	0,0,0,107,103,1,0,0,0,108,109,1,0,0,0,109,107,1,0,0,0,109,110,1,0,0,0,
  	110,127,1,0,0,0,111,112,5,31,0,0,112,113,5,3,0,0,113,127,3,8,4,0,114,
  	119,5,31,0,0,115,116,5,4,0,0,116,117,3,32,16,0,117,118,5,5,0,0,118,120,
  	1,0,0,0,119,115,1,0,0,0,120,121,1,0,0,0,121,119,1,0,0,0,121,122,1,0,0,
  	0,122,123,1,0,0,0,123,124,5,3,0,0,124,125,3,8,4,0,125,127,1,0,0,0,126,
  	101,1,0,0,0,126,102,1,0,0,0,126,111,1,0,0,0,126,114,1,0,0,0,127,13,1,
  	0,0,0,128,129,3,20,10,0,129,130,5,31,0,0,130,132,5,8,0,0,131,133,3,16,
  	8,0,132,131,1,0,0,0,132,133,1,0,0,0,133,134,1,0,0,0,134,135,5,9,0,0,135,
  	136,3,24,12,0,136,15,1,0,0,0,137,142,3,18,9,0,138,139,5,1,0,0,139,141,
  	3,18,9,0,140,138,1,0,0,0,141,144,1,0,0,0,142,140,1,0,0,0,142,143,1,0,
  	0,0,143,17,1,0,0,0,144,142,1,0,0,0,145,146,3,22,11,0,146,158,5,31,0,0,
  	147,148,5,4,0,0,148,155,5,5,0,0,149,150,5,4,0,0,150,151,3,32,16,0,151,
  	152,5,5,0,0,152,154,1,0,0,0,153,149,1,0,0,0,154,157,1,0,0,0,155,153,1,
  	0,0,0,155,156,1,0,0,0,156,159,1,0,0,0,157,155,1,0,0,0,158,147,1,0,0,0,
  	158,159,1,0,0,0,159,19,1,0,0,0,160,161,7,1,0,0,161,21,1,0,0,0,162,163,
  	7,0,0,0,163,23,1,0,0,0,164,169,5,6,0,0,165,168,3,2,1,0,166,168,3,26,13,
  	0,167,165,1,0,0,0,167,166,1,0,0,0,168,171,1,0,0,0,169,167,1,0,0,0,169,
  	170,1,0,0,0,170,172,1,0,0,0,171,169,1,0,0,0,172,173,5,7,0,0,173,25,1,
  	0,0,0,174,175,3,28,14,0,175,176,5,3,0,0,176,177,3,32,16,0,177,178,5,2,
  	0,0,178,226,1,0,0,0,179,180,5,31,0,0,180,181,5,8,0,0,181,186,3,32,16,
  	0,182,183,5,1,0,0,183,185,3,32,16,0,184,182,1,0,0,0,185,188,1,0,0,0,186,
  	184,1,0,0,0,186,187,1,0,0,0,187,189,1,0,0,0,188,186,1,0,0,0,189,190,5,
  	9,0,0,190,191,5,2,0,0,191,226,1,0,0,0,192,193,5,31,0,0,193,194,5,8,0,
  	0,194,195,5,9,0,0,195,226,5,2,0,0,196,226,3,24,12,0,197,198,5,10,0,0,
  	198,199,5,8,0,0,199,200,3,30,15,0,200,201,5,9,0,0,201,204,3,26,13,0,202,
  	203,5,11,0,0,203,205,3,26,13,0,204,202,1,0,0,0,204,205,1,0,0,0,205,226,
  	1,0,0,0,206,207,5,12,0,0,207,208,5,8,0,0,208,209,3,30,15,0,209,210,5,
  	9,0,0,210,211,3,26,13,0,211,226,1,0,0,0,212,213,5,35,0,0,213,226,5,2,
  	0,0,214,215,5,34,0,0,215,226,5,2,0,0,216,218,5,36,0,0,217,219,3,32,16,
  	0,218,217,1,0,0,0,218,219,1,0,0,0,219,220,1,0,0,0,220,226,5,2,0,0,221,
  	223,3,32,16,0,222,221,1,0,0,0,222,223,1,0,0,0,223,224,1,0,0,0,224,226,
  	5,2,0,0,225,174,1,0,0,0,225,179,1,0,0,0,225,192,1,0,0,0,225,196,1,0,0,
  	0,225,197,1,0,0,0,225,206,1,0,0,0,225,212,1,0,0,0,225,214,1,0,0,0,225,
  	216,1,0,0,0,225,222,1,0,0,0,226,27,1,0,0,0,227,238,5,31,0,0,228,233,5,
  	31,0,0,229,230,5,4,0,0,230,231,3,32,16,0,231,232,5,5,0,0,232,234,1,0,
  	0,0,233,229,1,0,0,0,234,235,1,0,0,0,235,233,1,0,0,0,235,236,1,0,0,0,236,
  	238,1,0,0,0,237,227,1,0,0,0,237,228,1,0,0,0,238,29,1,0,0,0,239,240,6,
  	15,-1,0,240,244,3,32,16,0,241,242,7,2,0,0,242,244,3,30,15,3,243,239,1,
  	0,0,0,243,241,1,0,0,0,244,259,1,0,0,0,245,246,10,6,0,0,246,247,7,3,0,
  	0,247,258,3,30,15,7,248,249,10,5,0,0,249,250,7,4,0,0,250,258,3,30,15,
  	6,251,252,10,2,0,0,252,253,5,26,0,0,253,258,3,30,15,3,254,255,10,1,0,
  	0,255,256,5,27,0,0,256,258,3,30,15,2,257,245,1,0,0,0,257,248,1,0,0,0,
  	257,251,1,0,0,0,257,254,1,0,0,0,258,261,1,0,0,0,259,257,1,0,0,0,259,260,
  	1,0,0,0,260,31,1,0,0,0,261,259,1,0,0,0,262,263,6,16,-1,0,263,264,7,5,
  	0,0,264,287,3,32,16,8,265,266,5,8,0,0,266,267,3,32,16,0,267,268,5,9,0,
  	0,268,287,1,0,0,0,269,287,3,34,17,0,270,287,3,28,14,0,271,272,5,31,0,
  	0,272,273,5,8,0,0,273,278,3,32,16,0,274,275,5,1,0,0,275,277,3,32,16,0,
  	276,274,1,0,0,0,277,280,1,0,0,0,278,276,1,0,0,0,278,279,1,0,0,0,279,281,
  	1,0,0,0,280,278,1,0,0,0,281,282,5,9,0,0,282,287,1,0,0,0,283,284,5,31,
  	0,0,284,285,5,8,0,0,285,287,5,9,0,0,286,262,1,0,0,0,286,265,1,0,0,0,286,
  	269,1,0,0,0,286,270,1,0,0,0,286,271,1,0,0,0,286,283,1,0,0,0,287,296,1,
  	0,0,0,288,289,10,7,0,0,289,290,7,6,0,0,290,295,3,32,16,8,291,292,10,6,
  	0,0,292,293,7,5,0,0,293,295,3,32,16,7,294,288,1,0,0,0,294,291,1,0,0,0,
  	295,298,1,0,0,0,296,294,1,0,0,0,296,297,1,0,0,0,297,33,1,0,0,0,298,296,
  	1,0,0,0,299,300,7,7,0,0,300,35,1,0,0,0,33,38,40,44,53,68,73,81,84,88,
  	96,109,121,126,132,142,155,158,167,169,186,204,218,222,225,235,237,243,
  	257,259,278,286,294,296
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  sysyparserParserStaticData = staticData.release();
}

}

SYSYParser::SYSYParser(TokenStream *input) : SYSYParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

SYSYParser::SYSYParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  SYSYParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *sysyparserParserStaticData->atn, sysyparserParserStaticData->decisionToDFA, sysyparserParserStaticData->sharedContextCache, options);
}

SYSYParser::~SYSYParser() {
  delete _interpreter;
}

const atn::ATN& SYSYParser::getATN() const {
  return *sysyparserParserStaticData->atn;
}

std::string SYSYParser::getGrammarFileName() const {
  return "SYSYParser.g4";
}

const std::vector<std::string>& SYSYParser::getRuleNames() const {
  return sysyparserParserStaticData->ruleNames;
}

const dfa::Vocabulary& SYSYParser::getVocabulary() const {
  return sysyparserParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SYSYParser::getSerializedATN() const {
  return sysyparserParserStaticData->serializedATN;
}


//----------------- CompilationUnitContext ------------------------------------------------------------------

SYSYParser::CompilationUnitContext::CompilationUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SYSYParser::DeclContext *> SYSYParser::CompilationUnitContext::decl() {
  return getRuleContexts<SYSYParser::DeclContext>();
}

SYSYParser::DeclContext* SYSYParser::CompilationUnitContext::decl(size_t i) {
  return getRuleContext<SYSYParser::DeclContext>(i);
}

std::vector<SYSYParser::FuncdefContext *> SYSYParser::CompilationUnitContext::funcdef() {
  return getRuleContexts<SYSYParser::FuncdefContext>();
}

SYSYParser::FuncdefContext* SYSYParser::CompilationUnitContext::funcdef(size_t i) {
  return getRuleContext<SYSYParser::FuncdefContext>(i);
}


size_t SYSYParser::CompilationUnitContext::getRuleIndex() const {
  return SYSYParser::RuleCompilationUnit;
}

void SYSYParser::CompilationUnitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompilationUnit(this);
}

void SYSYParser::CompilationUnitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompilationUnit(this);
}


std::any SYSYParser::CompilationUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitCompilationUnit(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::CompilationUnitContext* SYSYParser::compilationUnit() {
  CompilationUnitContext *_localctx = _tracker.createInstance<CompilationUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, SYSYParser::RuleCompilationUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(38); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(38);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(36);
        decl();
        break;
      }

      case 2: {
        setState(37);
        funcdef();
        break;
      }

      default:
        break;
      }
      setState(40); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1879056384) != 0));
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclContext ------------------------------------------------------------------

SYSYParser::DeclContext::DeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SYSYParser::ConstdeclContext* SYSYParser::DeclContext::constdecl() {
  return getRuleContext<SYSYParser::ConstdeclContext>(0);
}

SYSYParser::VardeclContext* SYSYParser::DeclContext::vardecl() {
  return getRuleContext<SYSYParser::VardeclContext>(0);
}


size_t SYSYParser::DeclContext::getRuleIndex() const {
  return SYSYParser::RuleDecl;
}

void SYSYParser::DeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDecl(this);
}

void SYSYParser::DeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDecl(this);
}


std::any SYSYParser::DeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitDecl(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::DeclContext* SYSYParser::decl() {
  DeclContext *_localctx = _tracker.createInstance<DeclContext>(_ctx, getState());
  enterRule(_localctx, 2, SYSYParser::RuleDecl);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(44);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SYSYParser::Const: {
        enterOuterAlt(_localctx, 1);
        setState(42);
        constdecl();
        break;
      }

      case SYSYParser::Int:
      case SYSYParser::Float: {
        enterOuterAlt(_localctx, 2);
        setState(43);
        vardecl();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstdeclContext ------------------------------------------------------------------

SYSYParser::ConstdeclContext::ConstdeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SYSYParser::ConstdeclContext::Const() {
  return getToken(SYSYParser::Const, 0);
}

std::vector<SYSYParser::ConstdefContext *> SYSYParser::ConstdeclContext::constdef() {
  return getRuleContexts<SYSYParser::ConstdefContext>();
}

SYSYParser::ConstdefContext* SYSYParser::ConstdeclContext::constdef(size_t i) {
  return getRuleContext<SYSYParser::ConstdefContext>(i);
}

tree::TerminalNode* SYSYParser::ConstdeclContext::SemiColon() {
  return getToken(SYSYParser::SemiColon, 0);
}

tree::TerminalNode* SYSYParser::ConstdeclContext::Int() {
  return getToken(SYSYParser::Int, 0);
}

tree::TerminalNode* SYSYParser::ConstdeclContext::Float() {
  return getToken(SYSYParser::Float, 0);
}

std::vector<tree::TerminalNode *> SYSYParser::ConstdeclContext::Comma() {
  return getTokens(SYSYParser::Comma);
}

tree::TerminalNode* SYSYParser::ConstdeclContext::Comma(size_t i) {
  return getToken(SYSYParser::Comma, i);
}


size_t SYSYParser::ConstdeclContext::getRuleIndex() const {
  return SYSYParser::RuleConstdecl;
}

void SYSYParser::ConstdeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConstdecl(this);
}

void SYSYParser::ConstdeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConstdecl(this);
}


std::any SYSYParser::ConstdeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitConstdecl(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::ConstdeclContext* SYSYParser::constdecl() {
  ConstdeclContext *_localctx = _tracker.createInstance<ConstdeclContext>(_ctx, getState());
  enterRule(_localctx, 4, SYSYParser::RuleConstdecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(46);
    match(SYSYParser::Const);
    setState(47);
    _la = _input->LA(1);
    if (!(_la == SYSYParser::Int

    || _la == SYSYParser::Float)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(48);
    constdef();
    setState(53);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SYSYParser::Comma) {
      setState(49);
      match(SYSYParser::Comma);
      setState(50);
      constdef();
      setState(55);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(56);
    match(SYSYParser::SemiColon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstdefContext ------------------------------------------------------------------

SYSYParser::ConstdefContext::ConstdefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SYSYParser::ConstdefContext::Identifier() {
  return getToken(SYSYParser::Identifier, 0);
}

tree::TerminalNode* SYSYParser::ConstdefContext::Assign() {
  return getToken(SYSYParser::Assign, 0);
}

SYSYParser::InitContext* SYSYParser::ConstdefContext::init() {
  return getRuleContext<SYSYParser::InitContext>(0);
}

std::vector<tree::TerminalNode *> SYSYParser::ConstdefContext::LeftBracket() {
  return getTokens(SYSYParser::LeftBracket);
}

tree::TerminalNode* SYSYParser::ConstdefContext::LeftBracket(size_t i) {
  return getToken(SYSYParser::LeftBracket, i);
}

std::vector<SYSYParser::ExpContext *> SYSYParser::ConstdefContext::exp() {
  return getRuleContexts<SYSYParser::ExpContext>();
}

SYSYParser::ExpContext* SYSYParser::ConstdefContext::exp(size_t i) {
  return getRuleContext<SYSYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SYSYParser::ConstdefContext::RightBracket() {
  return getTokens(SYSYParser::RightBracket);
}

tree::TerminalNode* SYSYParser::ConstdefContext::RightBracket(size_t i) {
  return getToken(SYSYParser::RightBracket, i);
}


size_t SYSYParser::ConstdefContext::getRuleIndex() const {
  return SYSYParser::RuleConstdef;
}

void SYSYParser::ConstdefContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConstdef(this);
}

void SYSYParser::ConstdefContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConstdef(this);
}


std::any SYSYParser::ConstdefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitConstdef(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::ConstdefContext* SYSYParser::constdef() {
  ConstdefContext *_localctx = _tracker.createInstance<ConstdefContext>(_ctx, getState());
  enterRule(_localctx, 6, SYSYParser::RuleConstdef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(73);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(58);
      match(SYSYParser::Identifier);
      setState(59);
      match(SYSYParser::Assign);
      setState(60);
      init();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(61);
      match(SYSYParser::Identifier);
      setState(66); 
      _errHandler->sync(this);
      _la = _input->LA(1);
      do {
        setState(62);
        match(SYSYParser::LeftBracket);
        setState(63);
        exp(0);
        setState(64);
        match(SYSYParser::RightBracket);
        setState(68); 
        _errHandler->sync(this);
        _la = _input->LA(1);
      } while (_la == SYSYParser::LeftBracket);
      setState(70);
      match(SYSYParser::Assign);
      setState(71);
      init();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InitContext ------------------------------------------------------------------

SYSYParser::InitContext::InitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SYSYParser::InitContext::LeftBrace() {
  return getToken(SYSYParser::LeftBrace, 0);
}

tree::TerminalNode* SYSYParser::InitContext::RightBrace() {
  return getToken(SYSYParser::RightBrace, 0);
}

std::vector<SYSYParser::InitContext *> SYSYParser::InitContext::init() {
  return getRuleContexts<SYSYParser::InitContext>();
}

SYSYParser::InitContext* SYSYParser::InitContext::init(size_t i) {
  return getRuleContext<SYSYParser::InitContext>(i);
}

std::vector<tree::TerminalNode *> SYSYParser::InitContext::Comma() {
  return getTokens(SYSYParser::Comma);
}

tree::TerminalNode* SYSYParser::InitContext::Comma(size_t i) {
  return getToken(SYSYParser::Comma, i);
}

SYSYParser::ExpContext* SYSYParser::InitContext::exp() {
  return getRuleContext<SYSYParser::ExpContext>(0);
}


size_t SYSYParser::InitContext::getRuleIndex() const {
  return SYSYParser::RuleInit;
}

void SYSYParser::InitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInit(this);
}

void SYSYParser::InitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInit(this);
}


std::any SYSYParser::InitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitInit(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::InitContext* SYSYParser::init() {
  InitContext *_localctx = _tracker.createInstance<InitContext>(_ctx, getState());
  enterRule(_localctx, 8, SYSYParser::RuleInit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(88);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SYSYParser::LeftBrace: {
        enterOuterAlt(_localctx, 1);
        setState(75);
        match(SYSYParser::LeftBrace);
        setState(84);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 15035531584) != 0)) {
          setState(76);
          init();
          setState(81);
          _errHandler->sync(this);
          _la = _input->LA(1);
          while (_la == SYSYParser::Comma) {
            setState(77);
            match(SYSYParser::Comma);
            setState(78);
            init();
            setState(83);
            _errHandler->sync(this);
            _la = _input->LA(1);
          }
        }
        setState(86);
        match(SYSYParser::RightBrace);
        break;
      }

      case SYSYParser::LeftParen:
      case SYSYParser::Plus:
      case SYSYParser::Minus:
      case SYSYParser::Identifier:
      case SYSYParser::FloatConst:
      case SYSYParser::IntConst: {
        enterOuterAlt(_localctx, 2);
        setState(87);
        exp(0);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VardeclContext ------------------------------------------------------------------

SYSYParser::VardeclContext::VardeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SYSYParser::VardefContext *> SYSYParser::VardeclContext::vardef() {
  return getRuleContexts<SYSYParser::VardefContext>();
}

SYSYParser::VardefContext* SYSYParser::VardeclContext::vardef(size_t i) {
  return getRuleContext<SYSYParser::VardefContext>(i);
}

tree::TerminalNode* SYSYParser::VardeclContext::SemiColon() {
  return getToken(SYSYParser::SemiColon, 0);
}

tree::TerminalNode* SYSYParser::VardeclContext::Int() {
  return getToken(SYSYParser::Int, 0);
}

tree::TerminalNode* SYSYParser::VardeclContext::Float() {
  return getToken(SYSYParser::Float, 0);
}

std::vector<tree::TerminalNode *> SYSYParser::VardeclContext::Comma() {
  return getTokens(SYSYParser::Comma);
}

tree::TerminalNode* SYSYParser::VardeclContext::Comma(size_t i) {
  return getToken(SYSYParser::Comma, i);
}


size_t SYSYParser::VardeclContext::getRuleIndex() const {
  return SYSYParser::RuleVardecl;
}

void SYSYParser::VardeclContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVardecl(this);
}

void SYSYParser::VardeclContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVardecl(this);
}


std::any SYSYParser::VardeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitVardecl(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::VardeclContext* SYSYParser::vardecl() {
  VardeclContext *_localctx = _tracker.createInstance<VardeclContext>(_ctx, getState());
  enterRule(_localctx, 10, SYSYParser::RuleVardecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(90);
    _la = _input->LA(1);
    if (!(_la == SYSYParser::Int

    || _la == SYSYParser::Float)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(91);
    vardef();
    setState(96);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SYSYParser::Comma) {
      setState(92);
      match(SYSYParser::Comma);
      setState(93);
      vardef();
      setState(98);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(99);
    match(SYSYParser::SemiColon);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VardefContext ------------------------------------------------------------------

SYSYParser::VardefContext::VardefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SYSYParser::VardefContext::Identifier() {
  return getToken(SYSYParser::Identifier, 0);
}

std::vector<tree::TerminalNode *> SYSYParser::VardefContext::LeftBracket() {
  return getTokens(SYSYParser::LeftBracket);
}

tree::TerminalNode* SYSYParser::VardefContext::LeftBracket(size_t i) {
  return getToken(SYSYParser::LeftBracket, i);
}

std::vector<SYSYParser::ExpContext *> SYSYParser::VardefContext::exp() {
  return getRuleContexts<SYSYParser::ExpContext>();
}

SYSYParser::ExpContext* SYSYParser::VardefContext::exp(size_t i) {
  return getRuleContext<SYSYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SYSYParser::VardefContext::RightBracket() {
  return getTokens(SYSYParser::RightBracket);
}

tree::TerminalNode* SYSYParser::VardefContext::RightBracket(size_t i) {
  return getToken(SYSYParser::RightBracket, i);
}

tree::TerminalNode* SYSYParser::VardefContext::Assign() {
  return getToken(SYSYParser::Assign, 0);
}

SYSYParser::InitContext* SYSYParser::VardefContext::init() {
  return getRuleContext<SYSYParser::InitContext>(0);
}


size_t SYSYParser::VardefContext::getRuleIndex() const {
  return SYSYParser::RuleVardef;
}

void SYSYParser::VardefContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVardef(this);
}

void SYSYParser::VardefContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVardef(this);
}


std::any SYSYParser::VardefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitVardef(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::VardefContext* SYSYParser::vardef() {
  VardefContext *_localctx = _tracker.createInstance<VardefContext>(_ctx, getState());
  enterRule(_localctx, 12, SYSYParser::RuleVardef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(126);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 12, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(101);
      match(SYSYParser::Identifier);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(102);
      match(SYSYParser::Identifier);
      setState(107); 
      _errHandler->sync(this);
      _la = _input->LA(1);
      do {
        setState(103);
        match(SYSYParser::LeftBracket);
        setState(104);
        exp(0);
        setState(105);
        match(SYSYParser::RightBracket);
        setState(109); 
        _errHandler->sync(this);
        _la = _input->LA(1);
      } while (_la == SYSYParser::LeftBracket);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(111);
      match(SYSYParser::Identifier);
      setState(112);
      match(SYSYParser::Assign);
      setState(113);
      init();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(114);
      match(SYSYParser::Identifier);
      setState(119); 
      _errHandler->sync(this);
      _la = _input->LA(1);
      do {
        setState(115);
        match(SYSYParser::LeftBracket);
        setState(116);
        exp(0);
        setState(117);
        match(SYSYParser::RightBracket);
        setState(121); 
        _errHandler->sync(this);
        _la = _input->LA(1);
      } while (_la == SYSYParser::LeftBracket);
      setState(123);
      match(SYSYParser::Assign);
      setState(124);
      init();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncdefContext ------------------------------------------------------------------

SYSYParser::FuncdefContext::FuncdefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SYSYParser::FunctypeContext* SYSYParser::FuncdefContext::functype() {
  return getRuleContext<SYSYParser::FunctypeContext>(0);
}

tree::TerminalNode* SYSYParser::FuncdefContext::Identifier() {
  return getToken(SYSYParser::Identifier, 0);
}

tree::TerminalNode* SYSYParser::FuncdefContext::LeftParen() {
  return getToken(SYSYParser::LeftParen, 0);
}

tree::TerminalNode* SYSYParser::FuncdefContext::RightParen() {
  return getToken(SYSYParser::RightParen, 0);
}

SYSYParser::BlockContext* SYSYParser::FuncdefContext::block() {
  return getRuleContext<SYSYParser::BlockContext>(0);
}

SYSYParser::FuncFparamsContext* SYSYParser::FuncdefContext::funcFparams() {
  return getRuleContext<SYSYParser::FuncFparamsContext>(0);
}


size_t SYSYParser::FuncdefContext::getRuleIndex() const {
  return SYSYParser::RuleFuncdef;
}

void SYSYParser::FuncdefContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuncdef(this);
}

void SYSYParser::FuncdefContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuncdef(this);
}


std::any SYSYParser::FuncdefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitFuncdef(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::FuncdefContext* SYSYParser::funcdef() {
  FuncdefContext *_localctx = _tracker.createInstance<FuncdefContext>(_ctx, getState());
  enterRule(_localctx, 14, SYSYParser::RuleFuncdef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(128);
    functype();
    setState(129);
    match(SYSYParser::Identifier);
    setState(130);
    match(SYSYParser::LeftParen);
    setState(132);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SYSYParser::Int

    || _la == SYSYParser::Float) {
      setState(131);
      funcFparams();
    }
    setState(134);
    match(SYSYParser::RightParen);
    setState(135);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFparamsContext ------------------------------------------------------------------

SYSYParser::FuncFparamsContext::FuncFparamsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SYSYParser::FuncFparamContext *> SYSYParser::FuncFparamsContext::funcFparam() {
  return getRuleContexts<SYSYParser::FuncFparamContext>();
}

SYSYParser::FuncFparamContext* SYSYParser::FuncFparamsContext::funcFparam(size_t i) {
  return getRuleContext<SYSYParser::FuncFparamContext>(i);
}

std::vector<tree::TerminalNode *> SYSYParser::FuncFparamsContext::Comma() {
  return getTokens(SYSYParser::Comma);
}

tree::TerminalNode* SYSYParser::FuncFparamsContext::Comma(size_t i) {
  return getToken(SYSYParser::Comma, i);
}


size_t SYSYParser::FuncFparamsContext::getRuleIndex() const {
  return SYSYParser::RuleFuncFparams;
}

void SYSYParser::FuncFparamsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuncFparams(this);
}

void SYSYParser::FuncFparamsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuncFparams(this);
}


std::any SYSYParser::FuncFparamsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitFuncFparams(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::FuncFparamsContext* SYSYParser::funcFparams() {
  FuncFparamsContext *_localctx = _tracker.createInstance<FuncFparamsContext>(_ctx, getState());
  enterRule(_localctx, 16, SYSYParser::RuleFuncFparams);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(137);
    funcFparam();
    setState(142);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SYSYParser::Comma) {
      setState(138);
      match(SYSYParser::Comma);
      setState(139);
      funcFparam();
      setState(144);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncFparamContext ------------------------------------------------------------------

SYSYParser::FuncFparamContext::FuncFparamContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SYSYParser::TypeIdContext* SYSYParser::FuncFparamContext::typeId() {
  return getRuleContext<SYSYParser::TypeIdContext>(0);
}

tree::TerminalNode* SYSYParser::FuncFparamContext::Identifier() {
  return getToken(SYSYParser::Identifier, 0);
}

std::vector<tree::TerminalNode *> SYSYParser::FuncFparamContext::LeftBracket() {
  return getTokens(SYSYParser::LeftBracket);
}

tree::TerminalNode* SYSYParser::FuncFparamContext::LeftBracket(size_t i) {
  return getToken(SYSYParser::LeftBracket, i);
}

std::vector<tree::TerminalNode *> SYSYParser::FuncFparamContext::RightBracket() {
  return getTokens(SYSYParser::RightBracket);
}

tree::TerminalNode* SYSYParser::FuncFparamContext::RightBracket(size_t i) {
  return getToken(SYSYParser::RightBracket, i);
}

std::vector<SYSYParser::ExpContext *> SYSYParser::FuncFparamContext::exp() {
  return getRuleContexts<SYSYParser::ExpContext>();
}

SYSYParser::ExpContext* SYSYParser::FuncFparamContext::exp(size_t i) {
  return getRuleContext<SYSYParser::ExpContext>(i);
}


size_t SYSYParser::FuncFparamContext::getRuleIndex() const {
  return SYSYParser::RuleFuncFparam;
}

void SYSYParser::FuncFparamContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuncFparam(this);
}

void SYSYParser::FuncFparamContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuncFparam(this);
}


std::any SYSYParser::FuncFparamContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitFuncFparam(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::FuncFparamContext* SYSYParser::funcFparam() {
  FuncFparamContext *_localctx = _tracker.createInstance<FuncFparamContext>(_ctx, getState());
  enterRule(_localctx, 18, SYSYParser::RuleFuncFparam);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(145);
    typeId();
    setState(146);
    match(SYSYParser::Identifier);
    setState(158);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SYSYParser::LeftBracket) {
      setState(147);
      match(SYSYParser::LeftBracket);
      setState(148);
      match(SYSYParser::RightBracket);
      setState(155);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SYSYParser::LeftBracket) {
        setState(149);
        match(SYSYParser::LeftBracket);
        setState(150);
        exp(0);
        setState(151);
        match(SYSYParser::RightBracket);
        setState(157);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctypeContext ------------------------------------------------------------------

SYSYParser::FunctypeContext::FunctypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SYSYParser::FunctypeContext::Void() {
  return getToken(SYSYParser::Void, 0);
}

tree::TerminalNode* SYSYParser::FunctypeContext::Int() {
  return getToken(SYSYParser::Int, 0);
}

tree::TerminalNode* SYSYParser::FunctypeContext::Float() {
  return getToken(SYSYParser::Float, 0);
}


size_t SYSYParser::FunctypeContext::getRuleIndex() const {
  return SYSYParser::RuleFunctype;
}

void SYSYParser::FunctypeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctype(this);
}

void SYSYParser::FunctypeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctype(this);
}


std::any SYSYParser::FunctypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitFunctype(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::FunctypeContext* SYSYParser::functype() {
  FunctypeContext *_localctx = _tracker.createInstance<FunctypeContext>(_ctx, getState());
  enterRule(_localctx, 20, SYSYParser::RuleFunctype);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(160);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1879048192) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeIdContext ------------------------------------------------------------------

SYSYParser::TypeIdContext::TypeIdContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SYSYParser::TypeIdContext::Int() {
  return getToken(SYSYParser::Int, 0);
}

tree::TerminalNode* SYSYParser::TypeIdContext::Float() {
  return getToken(SYSYParser::Float, 0);
}


size_t SYSYParser::TypeIdContext::getRuleIndex() const {
  return SYSYParser::RuleTypeId;
}

void SYSYParser::TypeIdContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTypeId(this);
}

void SYSYParser::TypeIdContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTypeId(this);
}


std::any SYSYParser::TypeIdContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitTypeId(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::TypeIdContext* SYSYParser::typeId() {
  TypeIdContext *_localctx = _tracker.createInstance<TypeIdContext>(_ctx, getState());
  enterRule(_localctx, 22, SYSYParser::RuleTypeId);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(162);
    _la = _input->LA(1);
    if (!(_la == SYSYParser::Int

    || _la == SYSYParser::Float)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

SYSYParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SYSYParser::BlockContext::LeftBrace() {
  return getToken(SYSYParser::LeftBrace, 0);
}

tree::TerminalNode* SYSYParser::BlockContext::RightBrace() {
  return getToken(SYSYParser::RightBrace, 0);
}

std::vector<SYSYParser::DeclContext *> SYSYParser::BlockContext::decl() {
  return getRuleContexts<SYSYParser::DeclContext>();
}

SYSYParser::DeclContext* SYSYParser::BlockContext::decl(size_t i) {
  return getRuleContext<SYSYParser::DeclContext>(i);
}

std::vector<SYSYParser::StmtContext *> SYSYParser::BlockContext::stmt() {
  return getRuleContexts<SYSYParser::StmtContext>();
}

SYSYParser::StmtContext* SYSYParser::BlockContext::stmt(size_t i) {
  return getRuleContext<SYSYParser::StmtContext>(i);
}


size_t SYSYParser::BlockContext::getRuleIndex() const {
  return SYSYParser::RuleBlock;
}

void SYSYParser::BlockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBlock(this);
}

void SYSYParser::BlockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBlock(this);
}


std::any SYSYParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::BlockContext* SYSYParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 24, SYSYParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(164);
    match(SYSYParser::LeftBrace);
    setState(169);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 136099935556) != 0)) {
      setState(167);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case SYSYParser::Const:
        case SYSYParser::Int:
        case SYSYParser::Float: {
          setState(165);
          decl();
          break;
        }

        case SYSYParser::SemiColon:
        case SYSYParser::LeftBrace:
        case SYSYParser::LeftParen:
        case SYSYParser::If:
        case SYSYParser::While:
        case SYSYParser::Plus:
        case SYSYParser::Minus:
        case SYSYParser::Identifier:
        case SYSYParser::FloatConst:
        case SYSYParser::IntConst:
        case SYSYParser::Break:
        case SYSYParser::Continue:
        case SYSYParser::Return: {
          setState(166);
          stmt();
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(171);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(172);
    match(SYSYParser::RightBrace);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StmtContext ------------------------------------------------------------------

SYSYParser::StmtContext::StmtContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SYSYParser::LvalContext* SYSYParser::StmtContext::lval() {
  return getRuleContext<SYSYParser::LvalContext>(0);
}

tree::TerminalNode* SYSYParser::StmtContext::Assign() {
  return getToken(SYSYParser::Assign, 0);
}

std::vector<SYSYParser::ExpContext *> SYSYParser::StmtContext::exp() {
  return getRuleContexts<SYSYParser::ExpContext>();
}

SYSYParser::ExpContext* SYSYParser::StmtContext::exp(size_t i) {
  return getRuleContext<SYSYParser::ExpContext>(i);
}

tree::TerminalNode* SYSYParser::StmtContext::SemiColon() {
  return getToken(SYSYParser::SemiColon, 0);
}

tree::TerminalNode* SYSYParser::StmtContext::Identifier() {
  return getToken(SYSYParser::Identifier, 0);
}

tree::TerminalNode* SYSYParser::StmtContext::LeftParen() {
  return getToken(SYSYParser::LeftParen, 0);
}

tree::TerminalNode* SYSYParser::StmtContext::RightParen() {
  return getToken(SYSYParser::RightParen, 0);
}

std::vector<tree::TerminalNode *> SYSYParser::StmtContext::Comma() {
  return getTokens(SYSYParser::Comma);
}

tree::TerminalNode* SYSYParser::StmtContext::Comma(size_t i) {
  return getToken(SYSYParser::Comma, i);
}

SYSYParser::BlockContext* SYSYParser::StmtContext::block() {
  return getRuleContext<SYSYParser::BlockContext>(0);
}

tree::TerminalNode* SYSYParser::StmtContext::If() {
  return getToken(SYSYParser::If, 0);
}

SYSYParser::CondContext* SYSYParser::StmtContext::cond() {
  return getRuleContext<SYSYParser::CondContext>(0);
}

std::vector<SYSYParser::StmtContext *> SYSYParser::StmtContext::stmt() {
  return getRuleContexts<SYSYParser::StmtContext>();
}

SYSYParser::StmtContext* SYSYParser::StmtContext::stmt(size_t i) {
  return getRuleContext<SYSYParser::StmtContext>(i);
}

tree::TerminalNode* SYSYParser::StmtContext::Else() {
  return getToken(SYSYParser::Else, 0);
}

tree::TerminalNode* SYSYParser::StmtContext::While() {
  return getToken(SYSYParser::While, 0);
}

tree::TerminalNode* SYSYParser::StmtContext::Continue() {
  return getToken(SYSYParser::Continue, 0);
}

tree::TerminalNode* SYSYParser::StmtContext::Break() {
  return getToken(SYSYParser::Break, 0);
}

tree::TerminalNode* SYSYParser::StmtContext::Return() {
  return getToken(SYSYParser::Return, 0);
}


size_t SYSYParser::StmtContext::getRuleIndex() const {
  return SYSYParser::RuleStmt;
}

void SYSYParser::StmtContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStmt(this);
}

void SYSYParser::StmtContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStmt(this);
}


std::any SYSYParser::StmtContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitStmt(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::StmtContext* SYSYParser::stmt() {
  StmtContext *_localctx = _tracker.createInstance<StmtContext>(_ctx, getState());
  enterRule(_localctx, 26, SYSYParser::RuleStmt);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(225);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(174);
      lval();
      setState(175);
      match(SYSYParser::Assign);
      setState(176);
      exp(0);
      setState(177);
      match(SYSYParser::SemiColon);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(179);
      match(SYSYParser::Identifier);
      setState(180);
      match(SYSYParser::LeftParen);
      setState(181);
      exp(0);
      setState(186);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SYSYParser::Comma) {
        setState(182);
        match(SYSYParser::Comma);
        setState(183);
        exp(0);
        setState(188);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(189);
      match(SYSYParser::RightParen);
      setState(190);
      match(SYSYParser::SemiColon);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(192);
      match(SYSYParser::Identifier);
      setState(193);
      match(SYSYParser::LeftParen);
      setState(194);
      match(SYSYParser::RightParen);
      setState(195);
      match(SYSYParser::SemiColon);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(196);
      block();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(197);
      match(SYSYParser::If);
      setState(198);
      match(SYSYParser::LeftParen);
      setState(199);
      cond(0);
      setState(200);
      match(SYSYParser::RightParen);
      setState(201);
      stmt();
      setState(204);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 20, _ctx)) {
      case 1: {
        setState(202);
        match(SYSYParser::Else);
        setState(203);
        stmt();
        break;
      }

      default:
        break;
      }
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(206);
      match(SYSYParser::While);
      setState(207);
      match(SYSYParser::LeftParen);
      setState(208);
      cond(0);
      setState(209);
      match(SYSYParser::RightParen);
      setState(210);
      stmt();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(212);
      match(SYSYParser::Continue);
      setState(213);
      match(SYSYParser::SemiColon);
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(214);
      match(SYSYParser::Break);
      setState(215);
      match(SYSYParser::SemiColon);
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(216);
      match(SYSYParser::Return);
      setState(218);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 15035531520) != 0)) {
        setState(217);
        exp(0);
      }
      setState(220);
      match(SYSYParser::SemiColon);
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(222);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 15035531520) != 0)) {
        setState(221);
        exp(0);
      }
      setState(224);
      match(SYSYParser::SemiColon);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LvalContext ------------------------------------------------------------------

SYSYParser::LvalContext::LvalContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SYSYParser::LvalContext::Identifier() {
  return getToken(SYSYParser::Identifier, 0);
}

std::vector<tree::TerminalNode *> SYSYParser::LvalContext::LeftBracket() {
  return getTokens(SYSYParser::LeftBracket);
}

tree::TerminalNode* SYSYParser::LvalContext::LeftBracket(size_t i) {
  return getToken(SYSYParser::LeftBracket, i);
}

std::vector<SYSYParser::ExpContext *> SYSYParser::LvalContext::exp() {
  return getRuleContexts<SYSYParser::ExpContext>();
}

SYSYParser::ExpContext* SYSYParser::LvalContext::exp(size_t i) {
  return getRuleContext<SYSYParser::ExpContext>(i);
}

std::vector<tree::TerminalNode *> SYSYParser::LvalContext::RightBracket() {
  return getTokens(SYSYParser::RightBracket);
}

tree::TerminalNode* SYSYParser::LvalContext::RightBracket(size_t i) {
  return getToken(SYSYParser::RightBracket, i);
}


size_t SYSYParser::LvalContext::getRuleIndex() const {
  return SYSYParser::RuleLval;
}

void SYSYParser::LvalContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLval(this);
}

void SYSYParser::LvalContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLval(this);
}


std::any SYSYParser::LvalContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitLval(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::LvalContext* SYSYParser::lval() {
  LvalContext *_localctx = _tracker.createInstance<LvalContext>(_ctx, getState());
  enterRule(_localctx, 28, SYSYParser::RuleLval);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    setState(237);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(227);
      match(SYSYParser::Identifier);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(228);
      match(SYSYParser::Identifier);
      setState(233); 
      _errHandler->sync(this);
      alt = 1;
      do {
        switch (alt) {
          case 1: {
                setState(229);
                match(SYSYParser::LeftBracket);
                setState(230);
                exp(0);
                setState(231);
                match(SYSYParser::RightBracket);
                break;
              }

        default:
          throw NoViableAltException(this);
        }
        setState(235); 
        _errHandler->sync(this);
        alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 24, _ctx);
      } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CondContext ------------------------------------------------------------------

SYSYParser::CondContext::CondContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SYSYParser::ExpContext* SYSYParser::CondContext::exp() {
  return getRuleContext<SYSYParser::ExpContext>(0);
}

std::vector<SYSYParser::CondContext *> SYSYParser::CondContext::cond() {
  return getRuleContexts<SYSYParser::CondContext>();
}

SYSYParser::CondContext* SYSYParser::CondContext::cond(size_t i) {
  return getRuleContext<SYSYParser::CondContext>(i);
}

tree::TerminalNode* SYSYParser::CondContext::Plus() {
  return getToken(SYSYParser::Plus, 0);
}

tree::TerminalNode* SYSYParser::CondContext::Minus() {
  return getToken(SYSYParser::Minus, 0);
}

tree::TerminalNode* SYSYParser::CondContext::Not() {
  return getToken(SYSYParser::Not, 0);
}

tree::TerminalNode* SYSYParser::CondContext::Less() {
  return getToken(SYSYParser::Less, 0);
}

tree::TerminalNode* SYSYParser::CondContext::Greater() {
  return getToken(SYSYParser::Greater, 0);
}

tree::TerminalNode* SYSYParser::CondContext::LessEqual() {
  return getToken(SYSYParser::LessEqual, 0);
}

tree::TerminalNode* SYSYParser::CondContext::GreaterEqual() {
  return getToken(SYSYParser::GreaterEqual, 0);
}

tree::TerminalNode* SYSYParser::CondContext::Equal() {
  return getToken(SYSYParser::Equal, 0);
}

tree::TerminalNode* SYSYParser::CondContext::NonEqual() {
  return getToken(SYSYParser::NonEqual, 0);
}

tree::TerminalNode* SYSYParser::CondContext::And() {
  return getToken(SYSYParser::And, 0);
}

tree::TerminalNode* SYSYParser::CondContext::Or() {
  return getToken(SYSYParser::Or, 0);
}


size_t SYSYParser::CondContext::getRuleIndex() const {
  return SYSYParser::RuleCond;
}

void SYSYParser::CondContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCond(this);
}

void SYSYParser::CondContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCond(this);
}


std::any SYSYParser::CondContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitCond(this);
  else
    return visitor->visitChildren(this);
}


SYSYParser::CondContext* SYSYParser::cond() {
   return cond(0);
}

SYSYParser::CondContext* SYSYParser::cond(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SYSYParser::CondContext *_localctx = _tracker.createInstance<CondContext>(_ctx, parentState);
  SYSYParser::CondContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 30;
  enterRecursionRule(_localctx, 30, SYSYParser::RuleCond, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(243);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 26, _ctx)) {
    case 1: {
      setState(240);
      exp(0);
      break;
    }

    case 2: {
      setState(241);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 36700160) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(242);
      cond(3);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(259);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(257);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 27, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<CondContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleCond);
          setState(245);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(246);
          _la = _input->LA(1);
          if (!((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 983040) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(247);
          cond(7);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<CondContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleCond);
          setState(248);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(249);
          _la = _input->LA(1);
          if (!(_la == SYSYParser::Equal

          || _la == SYSYParser::NonEqual)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(250);
          cond(6);
          break;
        }

        case 3: {
          _localctx = _tracker.createInstance<CondContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleCond);
          setState(251);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(252);
          match(SYSYParser::And);
          setState(253);
          cond(3);
          break;
        }

        case 4: {
          _localctx = _tracker.createInstance<CondContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleCond);
          setState(254);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(255);
          match(SYSYParser::Or);
          setState(256);
          cond(2);
          break;
        }

        default:
          break;
        } 
      }
      setState(261);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- ExpContext ------------------------------------------------------------------

SYSYParser::ExpContext::ExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SYSYParser::ExpContext *> SYSYParser::ExpContext::exp() {
  return getRuleContexts<SYSYParser::ExpContext>();
}

SYSYParser::ExpContext* SYSYParser::ExpContext::exp(size_t i) {
  return getRuleContext<SYSYParser::ExpContext>(i);
}

tree::TerminalNode* SYSYParser::ExpContext::Plus() {
  return getToken(SYSYParser::Plus, 0);
}

tree::TerminalNode* SYSYParser::ExpContext::Minus() {
  return getToken(SYSYParser::Minus, 0);
}

tree::TerminalNode* SYSYParser::ExpContext::LeftParen() {
  return getToken(SYSYParser::LeftParen, 0);
}

tree::TerminalNode* SYSYParser::ExpContext::RightParen() {
  return getToken(SYSYParser::RightParen, 0);
}

SYSYParser::NumberContext* SYSYParser::ExpContext::number() {
  return getRuleContext<SYSYParser::NumberContext>(0);
}

SYSYParser::LvalContext* SYSYParser::ExpContext::lval() {
  return getRuleContext<SYSYParser::LvalContext>(0);
}

tree::TerminalNode* SYSYParser::ExpContext::Identifier() {
  return getToken(SYSYParser::Identifier, 0);
}

std::vector<tree::TerminalNode *> SYSYParser::ExpContext::Comma() {
  return getTokens(SYSYParser::Comma);
}

tree::TerminalNode* SYSYParser::ExpContext::Comma(size_t i) {
  return getToken(SYSYParser::Comma, i);
}

tree::TerminalNode* SYSYParser::ExpContext::Multiply() {
  return getToken(SYSYParser::Multiply, 0);
}

tree::TerminalNode* SYSYParser::ExpContext::Divide() {
  return getToken(SYSYParser::Divide, 0);
}

tree::TerminalNode* SYSYParser::ExpContext::Modulo() {
  return getToken(SYSYParser::Modulo, 0);
}


size_t SYSYParser::ExpContext::getRuleIndex() const {
  return SYSYParser::RuleExp;
}

void SYSYParser::ExpContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExp(this);
}

void SYSYParser::ExpContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExp(this);
}


std::any SYSYParser::ExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitExp(this);
  else
    return visitor->visitChildren(this);
}


SYSYParser::ExpContext* SYSYParser::exp() {
   return exp(0);
}

SYSYParser::ExpContext* SYSYParser::exp(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SYSYParser::ExpContext *_localctx = _tracker.createInstance<ExpContext>(_ctx, parentState);
  SYSYParser::ExpContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 32;
  enterRecursionRule(_localctx, 32, SYSYParser::RuleExp, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(286);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      setState(263);
      _la = _input->LA(1);
      if (!(_la == SYSYParser::Plus

      || _la == SYSYParser::Minus)) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(264);
      exp(8);
      break;
    }

    case 2: {
      setState(265);
      match(SYSYParser::LeftParen);
      setState(266);
      exp(0);
      setState(267);
      match(SYSYParser::RightParen);
      break;
    }

    case 3: {
      setState(269);
      number();
      break;
    }

    case 4: {
      setState(270);
      lval();
      break;
    }

    case 5: {
      setState(271);
      match(SYSYParser::Identifier);
      setState(272);
      match(SYSYParser::LeftParen);
      setState(273);
      exp(0);
      setState(278);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == SYSYParser::Comma) {
        setState(274);
        match(SYSYParser::Comma);
        setState(275);
        exp(0);
        setState(280);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
      setState(281);
      match(SYSYParser::RightParen);
      break;
    }

    case 6: {
      setState(283);
      match(SYSYParser::Identifier);
      setState(284);
      match(SYSYParser::LeftParen);
      setState(285);
      match(SYSYParser::RightParen);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(296);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(294);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<ExpContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExp);
          setState(288);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(289);
          _la = _input->LA(1);
          if (!((((_la & ~ 0x3fULL) == 0) &&
            ((1ULL << _la) & 29360128) != 0))) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(290);
          exp(8);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<ExpContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExp);
          setState(291);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(292);
          _la = _input->LA(1);
          if (!(_la == SYSYParser::Plus

          || _la == SYSYParser::Minus)) {
          _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(293);
          exp(7);
          break;
        }

        default:
          break;
        } 
      }
      setState(298);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- NumberContext ------------------------------------------------------------------

SYSYParser::NumberContext::NumberContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SYSYParser::NumberContext::IntConst() {
  return getToken(SYSYParser::IntConst, 0);
}

tree::TerminalNode* SYSYParser::NumberContext::FloatConst() {
  return getToken(SYSYParser::FloatConst, 0);
}


size_t SYSYParser::NumberContext::getRuleIndex() const {
  return SYSYParser::RuleNumber;
}

void SYSYParser::NumberContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNumber(this);
}

void SYSYParser::NumberContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SYSYParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNumber(this);
}


std::any SYSYParser::NumberContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<SYSYParserVisitor*>(visitor))
    return parserVisitor->visitNumber(this);
  else
    return visitor->visitChildren(this);
}

SYSYParser::NumberContext* SYSYParser::number() {
  NumberContext *_localctx = _tracker.createInstance<NumberContext>(_ctx, getState());
  enterRule(_localctx, 34, SYSYParser::RuleNumber);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(299);
    _la = _input->LA(1);
    if (!(_la == SYSYParser::FloatConst

    || _la == SYSYParser::IntConst)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool SYSYParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 15: return condSempred(antlrcpp::downCast<CondContext *>(context), predicateIndex);
    case 16: return expSempred(antlrcpp::downCast<ExpContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool SYSYParser::condSempred(CondContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 6);
    case 1: return precpred(_ctx, 5);
    case 2: return precpred(_ctx, 2);
    case 3: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool SYSYParser::expSempred(ExpContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 4: return precpred(_ctx, 7);
    case 5: return precpred(_ctx, 6);

  default:
    break;
  }
  return true;
}

void SYSYParser::initialize() {
  ::antlr4::internal::call_once(sysyparserParserOnceFlag, sysyparserParserInitialize);
}
