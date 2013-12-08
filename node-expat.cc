#include <nan.h>
extern "C" {
#include <expat.h>
}

using namespace v8;
using namespace node;

class Parser : public ObjectWrap {
public:
  static void Initialize(Handle<Object> target)
  {
    NanScope();
    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    t->InstanceTemplate()->SetInternalFieldCount(1);

    NODE_SET_PROTOTYPE_METHOD(t, "parse", Parse);
    NODE_SET_PROTOTYPE_METHOD(t, "setEncoding", SetEncoding);
    NODE_SET_PROTOTYPE_METHOD(t, "getError", GetError);
    NODE_SET_PROTOTYPE_METHOD(t, "stop", Stop);
    NODE_SET_PROTOTYPE_METHOD(t, "resume", Resume);
    NODE_SET_PROTOTYPE_METHOD(t, "reset", Reset);
    NODE_SET_PROTOTYPE_METHOD(t, "getCurrentLineNumber", GetCurrentLineNumber);
    NODE_SET_PROTOTYPE_METHOD(t, "getCurrentColumnNumber", GetCurrentColumnNumber);
    NODE_SET_PROTOTYPE_METHOD(t, "getCurrentByteIndex", GetCurrentByteIndex);

    target->Set(NanSymbol("Parser"), t->GetFunction());
  }

protected:
  /*** Constructor ***/

  static NAN_METHOD(New)
  {
    NanScope();
    XML_Char *encoding = NULL;
    if (args.Length() == 1 && args[0]->IsString())
      {
        encoding = new XML_Char[32];
        NanFromV8String(args[0], Nan::ASCII, NULL, encoding, 32, 0);
      }

    Parser *parser = new Parser(encoding);
    if (encoding)
      delete[] encoding;
    parser->Wrap(args.This());
    NanReturnValue(args.This());
  }

  Parser(const XML_Char *encoding)
    : ObjectWrap()
  {
    parser = XML_ParserCreate(encoding);
    assert(parser != NULL);

    attachHandlers();
  }

  ~Parser()
  {
    XML_ParserFree(parser);
  }

  void attachHandlers()
  {
    XML_SetUserData(parser, this);
    XML_SetElementHandler(parser, StartElement, EndElement);
    XML_SetCharacterDataHandler(parser, Text);
    XML_SetCdataSectionHandler(parser, StartCdata, EndCdata);
    XML_SetProcessingInstructionHandler(parser, ProcessingInstruction);
    XML_SetCommentHandler(parser, Comment);
    XML_SetXmlDeclHandler(parser, XmlDecl);
    XML_SetEntityDeclHandler(parser, EntityDecl);
  }
    
  /*** parse() ***/

  static NAN_METHOD(Parse)
  {
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());
    NanScope();
    Local<String> str;
    int isFinal = 0;

    /* Argument 2: isFinal :: Bool */
    if (args.Length() >= 2)
      {
        isFinal = args[1]->IsTrue();
      }

    /* Argument 1: buf :: String or Buffer */
    if (args.Length() >= 1 && args[0]->IsString())
      {
        str = args[0]->ToString();
        NanReturnValue(parser->parseString(**str, isFinal) ? True() : False());
      }
    else if (args.Length() >= 1 && args[0]->IsObject())
      {
        Local<Object> obj = args[0]->ToObject();
        if (Buffer::HasInstance(obj))
        {
          NanReturnValue(parser->parseBuffer(obj, isFinal) ? True() : False());
        }
        else
        {
          NanThrowTypeError("Parse buffer must be String or Buffer");
          NanReturnUndefined();
        }
      }
    else {
      NanThrowTypeError("Parse buffer must be String or Buffer");
      NanReturnUndefined();
    }
  }

  /** Parse a v8 String by first writing it to the expat parser's
      buffer */
  bool parseString(String &str, int isFinal)
  {
    int len = str.Utf8Length();
    if (len == 0)
      return true;

    void *buf = XML_GetBuffer(parser, len);
    assert(buf != NULL);
    assert(str.WriteUtf8(static_cast<char *>(buf), len) == len);

    return XML_ParseBuffer(parser, len, isFinal) != XML_STATUS_ERROR;
  }

  /** Parse a node.js Buffer directly */
  bool parseBuffer(Local<Object> buffer, int isFinal)
  {
    return XML_Parse(parser, Buffer::Data(buffer), Buffer::Length(buffer), isFinal) != XML_STATUS_ERROR;
  }

  /*** setEncoding() ***/

  static NAN_METHOD(SetEncoding)
  {
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());
    NanScope();

    if (args.Length() == 1 && args[0]->IsString())
      {
        XML_Char *encoding = new XML_Char[32];
        NanFromV8String(args[0], Nan::ASCII, NULL, encoding, 32, 0);

        int status = parser->setEncoding(encoding);

        delete[] encoding;

        NanReturnValue(status ? True() : False());
      }
    else
      NanReturnValue(False());
  }

  int setEncoding(XML_Char *encoding)
  {
    return XML_SetEncoding(parser, encoding) != 0;
  }

  /*** getError() ***/

  static NAN_METHOD(GetError)
  {
    NanScope();
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());

    const XML_LChar *error = parser->getError();
    if (error)
      NanReturnValue(String::New(error));
    else
      NanReturnValue(Null());
  }
  
  /*** stop() ***/

  static NAN_METHOD(Stop)
  {
    NanScope();
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());

    int status = parser->stop();
    
    NanReturnValue(status ? True() : False());
  }

  int stop()
  {
    return XML_StopParser(parser, XML_TRUE) != 0;
  }
  
  /*** resume() ***/

  static NAN_METHOD(Resume)
  {
    NanScope();
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());

    int status = parser->resume();
    
    NanReturnValue(status ? True() : False());
  }

  int resume()
  {
    return XML_ResumeParser(parser) != 0;
  }
  
  static NAN_METHOD(Reset)
  {
    NanScope();
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());
    XML_Char *encoding = NULL;
    if (args.Length() == 1 && args[0]->IsString())
      {
        encoding = new XML_Char[32];
        NanFromV8String(args[0], Nan::ASCII, NULL, encoding, 32, 0);
      }

    int status = parser->reset(encoding);
    if (status) 
      parser->attachHandlers();
    NanReturnValue(status ? True() : False());
  }

  int reset(XML_Char *encoding)
  {
      return XML_ParserReset(parser, encoding) != 0;
  }
  const XML_LChar *getError()
  {
    enum XML_Error code;
    code = XML_GetErrorCode(parser);
    return XML_ErrorString(code);
  }

  static NAN_METHOD(GetCurrentLineNumber)
  {
    NanScope();
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());

    NanReturnValue(Integer::NewFromUnsigned(parser->getCurrentLineNumber()));
  }

  uint32_t getCurrentLineNumber()
  {
    return XML_GetCurrentLineNumber(parser);
  }

  static NAN_METHOD(GetCurrentColumnNumber)
  {
    NanScope();
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());

    NanReturnValue(Integer::NewFromUnsigned(parser->getCurrentColumnNumber()));
  }

  uint32_t getCurrentColumnNumber()
  {
    return XML_GetCurrentColumnNumber(parser);
  }

  static NAN_METHOD(GetCurrentByteIndex)
  {
    NanScope();
    Parser *parser = ObjectWrap::Unwrap<Parser>(args.This());

    NanReturnValue(Integer::New(parser->getCurrentByteIndex()));
  }

  int32_t getCurrentByteIndex()
  {
    return XML_GetCurrentByteIndex(parser);
  }

private:
  /* expat instance */
  XML_Parser parser;

  /* no default ctor */
  Parser();
        
  /*** SAX callbacks ***/
  /* Should a local HandleScope be used in those callbacks? */

  static void StartElement(void *userData,
                           const XML_Char *name, const XML_Char **atts)
  {
    NanScope();
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Collect atts into JS object */
    Local<Object> attr = Object::New();
    for(const XML_Char **atts1 = atts; *atts1; atts1 += 2)
      attr->Set(String::New(atts1[0]), String::New(atts1[1]));

    /* Trigger event */
    Handle<Value> argv[3] = { NanSymbol("startElement"),
                              String::New(name),
                              attr };
    parser->Emit(3, argv);
  }

  static void EndElement(void *userData,
                         const XML_Char *name)
  {
    NanScope();
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Handle<Value> argv[2] = { NanSymbol("endElement"), String::New(name) };
    parser->Emit(2, argv);
  }
  
  static void StartCdata(void *userData)
  {
    NanScope();
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Handle<Value> argv[1] = { NanSymbol("startCdata") };
    parser->Emit(1, argv);
  }

  static void EndCdata(void *userData)
  {
    NanScope();
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Handle<Value> argv[1] = { NanSymbol("endCdata") };
    parser->Emit(1, argv);
  }

  static void Text(void *userData,
                   const XML_Char *s, int len)
  {
    NanScope();
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Handle<Value> argv[2] = { NanSymbol("text"),
                              String::New(s, len) };
    parser->Emit(2, argv);
  }

  static void ProcessingInstruction(void *userData,
                                    const XML_Char *target, const XML_Char *data)
  {
    NanScope();
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Handle<Value> argv[3] = { NanSymbol("processingInstruction"),
                              String::New(target),
                              String::New(data) };
    parser->Emit(3, argv);
  }

  static void Comment(void *userData,
                      const XML_Char *data)
  {
    NanScope();
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Handle<Value> argv[2] = { NanSymbol("comment"), String::New(data) };
    parser->Emit(2, argv);
  }

  static void XmlDecl(void *userData,
                      const XML_Char *version, const XML_Char *encoding,
                      int standalone)
  {
    NanScope();
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[4] = { NanSymbol("xmlDecl"),
                              version ? NanNewLocal<Value>(String::New(version))
                                      : NanNewLocal<Value>(Null()),
                              encoding ? NanNewLocal<Value>(String::New(encoding))
                                      : NanNewLocal<Value>(Null()),
                              NanNewLocal<Value>(Boolean::New(standalone)) };
    parser->Emit(4, argv);
  }

  static void EntityDecl(void *userData, const XML_Char *entityName, int is_parameter_entity,
                         const XML_Char *value, int value_length, const XML_Char *base,
                         const XML_Char *systemId, const XML_Char *publicId, const XML_Char *notationName)
  {
    NanScope();
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[8] = { NanSymbol("entityDecl"),
                              entityName ? NanNewLocal<Value>(String::New(entityName))
                                    : NanNewLocal<Value>(Null()),
                              NanNewLocal<Value>(Boolean::New(is_parameter_entity)),
                              value ? NanNewLocal<Value>(String::New(value, value_length))
                                    : NanNewLocal<Value>(Null()),
                              base ? NanNewLocal<Value>(String::New(base))
                                    : NanNewLocal<Value>(Null()),
                              systemId ? NanNewLocal<Value>(String::New(systemId))
                                    : NanNewLocal<Value>(Null()),
                              publicId ? NanNewLocal<Value>(String::New(publicId))
                                    : NanNewLocal<Value>(Null()),
                              notationName ? NanNewLocal<Value>(String::New(notationName))
                                    : NanNewLocal<Value>(Null())
    };
    parser->Emit(8, argv);
  }

  void Emit(int argc, Handle<Value> argv[])
  {
    NanScope();

    Handle<Object> handle = NanObjectWrapHandle(this);
    Local<Function> emit = handle->Get(NanSymbol("emit")).As<Function>();
    emit->Call(handle, argc, argv);
  }
};

extern "C" {
  static void init (Handle<Object> target)
  {
    Parser::Initialize(target);
  }
  //Changed the name cause I couldn't load the module with - in their names
  NODE_MODULE(node_expat, init);
};

