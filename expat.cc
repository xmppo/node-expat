#include <node.h>
#include <node_events.h>
extern "C" {
#include <expat.h>
}

using namespace v8;
using namespace node;

static Persistent<String> sym_startElement, sym_endElement, sym_text;

class Parser : public EventEmitter {
public:
  static void Initialize(Handle<Object> target)
  {
    HandleScope scope;
    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    t->Inherit(EventEmitter::constructor_template);
    t->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(t, "parse", Parse);

    target->Set(String::NewSymbol("Parser"), t->GetFunction());

    sym_startElement = NODE_PSYMBOL("startElement");
    sym_endElement = NODE_PSYMBOL("endElement");
    sym_text = NODE_PSYMBOL("text");
  }

protected:
  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;

    Parser *parser = new Parser();
    parser->Wrap(args.This());
    
    return args.This();
  }

  Parser()
    : EventEmitter()
  {
    parser = XML_ParserCreate(/*encoding*/ "UTF-8");
    assert(parser != NULL);

    XML_SetUserData(parser, this);
    XML_SetElementHandler(parser, StartElement, EndElement);
    XML_SetCharacterDataHandler(parser, Text);
  }

  ~Parser()
  {
    XML_ParserFree(parser);
  }

  static Handle<Value> Parse(const Arguments& args)
  {
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());
    HandleScope scope;
    Local<String> str;
    int isFinal = 0;

    /* Argument 1: buf :: String */
    if (args.Length() >= 1 && args[0]->IsString())
      {
        str = args[0]->ToString();
      }
    else
      return scope.Close(False());

    /* Argument 2: isFinal :: Bool */
    if (args.Length() >= 2)
      {
        isFinal = args[1]->IsTrue();
      }

    if (parser->parse(**str, isFinal))
      return scope.Close(False());
    else
      return scope.Close(True());
  }

  bool parse(String &str, int isFinal)
  {
    int len = str.Utf8Length();
    void *buf = XML_GetBuffer(parser, len);
    assert(buf != NULL);
    assert(str.WriteUtf8(static_cast<char *>(buf), len) == len);
    assert(XML_ParseBuffer(parser, len, isFinal) != XML_STATUS_ERROR);

    return true;
  }

private:
  XML_Parser parser;

  /*** SAX callbacks ***/

  static void StartElement(void *userData,
                           const XML_Char *name, const XML_Char **atts)
  {
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Collect atts into JS object */
    Local<Object> attr = Object::New();
    for(const XML_Char **atts1 = atts; *atts1; atts1 += 2)
      attr->Set(String::New(atts1[0]), String::New(atts1[1]));

    /* Trigger event */
    Handle<Value> argv[2] = { String::New(name), attr };
    parser->Emit(sym_startElement, 2, argv);
  }

  static void EndElement(void *userData,
                         const XML_Char *name)
  {
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Handle<Value> argv[1] = { String::New(name) };
    parser->Emit(sym_endElement, 1, argv);
  }

  static void Text(void *userData,
                   const XML_Char *s, int len)
  {
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Handle<Value> argv[1] = { String::New(s, len) };
    parser->Emit(sym_text, 1, argv);
  }
};



extern "C" void init(Handle<Object> target)
{
  HandleScope scope;
  Parser::Initialize(target);
}
