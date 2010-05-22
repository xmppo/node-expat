#include <node.h>
#include <node_events.h>
extern "C" {
#include <expat.h>
}

using namespace v8;
using namespace node;

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
    printf("initialized %i\n", t->GetFunction()->IsFunction());
  }

protected:
  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;
    printf("new\n");

    Parser *parser = new Parser();
    parser->Wrap(args.This());
    
    printf("newed\n");
    return args.This();
  }

  Parser()
    : EventEmitter()
  {
    parser = XML_ParserCreate(/*encoding*/ "UTF-8");
    assert(parser != NULL);
  }

  ~Parser()
  {
    //assert(parser == NULL);
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
    assert(XML_ParseBuffer(parser, len, isFinal) != 0);

    return true;
  }

private:
  XML_Parser parser;
};


extern "C" void init(Handle<Object> target)
{
  HandleScope scope;
  Parser::Initialize(target);
}
