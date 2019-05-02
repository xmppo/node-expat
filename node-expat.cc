#include <nan.h>
extern "C" {
#include <expat.h>
}

using namespace v8;
using namespace node;

class Parser : public Nan::ObjectWrap {
public:
  static void Initialize(Local<Object> target)
  {
    Nan::HandleScope scope;
    Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);

    t->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(t, "parse", Parse);
    Nan::SetPrototypeMethod(t, "setEncoding", SetEncoding);
    Nan::SetPrototypeMethod(t, "setUnknownEncoding", SetUnknownEncoding);
    Nan::SetPrototypeMethod(t, "getError", GetError);
    Nan::SetPrototypeMethod(t, "stop", Stop);
    Nan::SetPrototypeMethod(t, "resume", Resume);
    Nan::SetPrototypeMethod(t, "reset", Reset);
    Nan::SetPrototypeMethod(t, "getCurrentLineNumber", GetCurrentLineNumber);
    Nan::SetPrototypeMethod(t, "getCurrentColumnNumber", GetCurrentColumnNumber);
    Nan::SetPrototypeMethod(t, "getCurrentByteIndex", GetCurrentByteIndex);

    Nan::Set(target, Nan::New("Parser").ToLocalChecked(), Nan::GetFunction(t).ToLocalChecked());
  }

protected:
  /*** Constructor ***/

  static NAN_METHOD(New)
  {
    Nan::HandleScope scope;
    XML_Char *encoding = NULL;
    if (info.Length() == 1 && info[0]->IsString())
      {
        Nan::Utf8String encodingArg(info[0]);
        encoding = new XML_Char[encodingArg.length() + 1];
        strcpy(encoding, *encodingArg);
      }

    Parser *parser = new Parser(encoding);
    if (encoding)
      delete[] encoding;
    parser->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }

  Parser(const XML_Char *encoding)
    : Nan::ObjectWrap()
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
    XML_SetUnknownEncodingHandler(parser, UnknownEncoding, this);
  }

  /*** parse() ***/

  static NAN_METHOD(Parse)
  {
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());
    Nan::HandleScope scope;
    int isFinal = 0;

    /* Argument 2: isFinal :: Bool */
    if (info.Length() >= 2)
      {
        isFinal = info[1]->IsTrue();
      }

    /* Argument 1: buf :: String or Buffer */
    if (info.Length() >= 1 && info[0]->IsString())
      {
        Local<String> str = Nan::To<String>(info[0]).ToLocalChecked();
        info.GetReturnValue().Set(parser->parseString(str, isFinal) ? Nan::True() : Nan::False());
      }
    else if (info.Length() >= 1 && info[0]->IsObject())
      {
        Local<Object> obj = Nan::To<Object>(info[0]).ToLocalChecked();
        if (Buffer::HasInstance(obj))
        {
          info.GetReturnValue().Set(parser->parseBuffer(obj, isFinal) ? Nan::True() : Nan::False());
        }
        else
        {
          Nan::ThrowTypeError("Parse buffer must be String or Buffer");
          return;
        }
      }
    else {
      Nan::ThrowTypeError("Parse buffer must be String or Buffer");
      return;
    }
  }

  /** Parse a v8 String by first writing it to the expat parser's
      buffer */
  bool parseString(Local<String> str, int isFinal)
  {
    int len = Nan::Utf8String(str).length();
    if (len == 0)
      return true;


    void *buf = XML_GetBuffer(parser, len);
    assert(buf != NULL);
    assert(Nan::DecodeWrite(static_cast<char *>(buf), len, str, Nan::Encoding::UTF8) == len);

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
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());
    Nan::HandleScope scope;

    if (info.Length() == 1 && info[0]->IsString())
      {
        Nan::Utf8String encoding(info[0]);
        int status = parser->setEncoding(*encoding);

        info.GetReturnValue().Set(status ? Nan::True() : Nan::False());
      }
    else
      info.GetReturnValue().Set(Nan::False());
  }

  int setEncoding(XML_Char *encoding)
  {
    return XML_SetEncoding(parser, encoding) != 0;
  }

  /*** getError() ***/

  static NAN_METHOD(GetError)
  {
    Nan::HandleScope scope;
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());

    const XML_LChar *error = parser->getError();
    if (error)
      info.GetReturnValue().Set(Nan::New(error).ToLocalChecked());
    else
      info.GetReturnValue().Set(Nan::Null());
  }

  /*** stop() ***/

  static NAN_METHOD(Stop)
  {
    Nan::HandleScope scope;
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());

    int status = parser->stop();

    info.GetReturnValue().Set(status ? Nan::True() : Nan::False());
  }

  int stop()
  {
    return XML_StopParser(parser, XML_TRUE) != 0;
  }

  /*** resume() ***/

  static NAN_METHOD(Resume)
  {
    Nan::HandleScope scope;
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());

    int status = parser->resume();

    info.GetReturnValue().Set(status ? Nan::True() : Nan::False());
  }

  int resume()
  {
    return XML_ResumeParser(parser) != 0;
  }

  static NAN_METHOD(Reset)
  {
    Nan::HandleScope scope;
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());
    XML_Char *encoding = NULL;
    if (info.Length() == 1 && info[0]->IsString())
      {
        Nan::Utf8String encodingArg(info[0]);
        encoding = new XML_Char[encodingArg.length() + 1];
        strcpy(encoding, *encodingArg);
      }

    int status = parser->reset(encoding);
    if (encoding)
      delete[] encoding;
    if (status)
      parser->attachHandlers();
    info.GetReturnValue().Set(status ? Nan::True() : Nan::False());
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
    Nan::HandleScope scope;
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());

    info.GetReturnValue().Set(Nan::New(parser->getCurrentLineNumber()));
  }

  uint32_t getCurrentLineNumber()
  {
    return XML_GetCurrentLineNumber(parser);
  }

  static NAN_METHOD(GetCurrentColumnNumber)
  {
    Nan::HandleScope scope;
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());

    info.GetReturnValue().Set(Nan::New(parser->getCurrentColumnNumber()));
  }

  uint32_t getCurrentColumnNumber()
  {
    return XML_GetCurrentColumnNumber(parser);
  }

  static NAN_METHOD(GetCurrentByteIndex)
  {
    Nan::HandleScope scope;
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());

    info.GetReturnValue().Set(Nan::New(parser->getCurrentByteIndex()));
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
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Collect atts into JS object */
    Local<Object> attr = Nan::New<Object>();
    for(const XML_Char **atts1 = atts; *atts1; atts1 += 2)
      Nan::Set(attr, Nan::New(atts1[0]).ToLocalChecked(), Nan::New(atts1[1]).ToLocalChecked());

    /* Trigger event */
    Local<Value> argv[3] = { Nan::New("startElement").ToLocalChecked(),
                              Nan::New(name).ToLocalChecked(),
                              attr };
    parser->Emit(3, argv);
  }

  static void EndElement(void *userData,
                         const XML_Char *name)
  {
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[2] = { Nan::New("endElement").ToLocalChecked(), Nan::New(name).ToLocalChecked() };
    parser->Emit(2, argv);
  }

  static void StartCdata(void *userData)
  {
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[1] = { Nan::New("startCdata").ToLocalChecked() };
    parser->Emit(1, argv);
  }

  static void EndCdata(void *userData)
  {
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[1] = { Nan::New("endCdata").ToLocalChecked() };
    parser->Emit(1, argv);
  }

  static void Text(void *userData,
                   const XML_Char *s, int len)
  {
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[2] = { Nan::New("text").ToLocalChecked(),
                              Nan::New(s, len).ToLocalChecked() };
    parser->Emit(2, argv);
  }

  static void ProcessingInstruction(void *userData,
                                    const XML_Char *target, const XML_Char *data)
  {
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[3] = { Nan::New("processingInstruction").ToLocalChecked(),
                              Nan::New(target).ToLocalChecked(),
                              Nan::New(data).ToLocalChecked() };
    parser->Emit(3, argv);
  }

  static void Comment(void *userData,
                      const XML_Char *data)
  {
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[2] = { Nan::New("comment").ToLocalChecked(), Nan::New(data).ToLocalChecked() };
    parser->Emit(2, argv);
  }

  static void XmlDecl(void *userData,
                      const XML_Char *version, const XML_Char *encoding,
                      int standalone)
  {
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[4];

                    argv[0] = Nan::New("xmlDecl").ToLocalChecked();
    if (version)    argv[1] = Nan::New(version).ToLocalChecked();
    else            argv[1] = Nan::Null();
    if (encoding)   argv[2] = Nan::New(encoding).ToLocalChecked();
    else            argv[2] = Nan::Null();
    if (standalone) argv[3] = Nan::True();
    else            argv[3] = Nan::False();

    parser->Emit(4, argv);
  }

  static void EntityDecl(void *userData, const XML_Char *entityName, int is_parameter_entity,
                         const XML_Char *value, int value_length, const XML_Char *base,
                         const XML_Char *systemId, const XML_Char *publicId, const XML_Char *notationName)
  {
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(userData);

    /* Trigger event */
    Local<Value> argv[8];

                             argv[0] = Nan::New("entityDecl").ToLocalChecked();
    if (entityName)          argv[1] = Nan::New(entityName).ToLocalChecked();
    else                     argv[1] = Nan::Null();
    if (is_parameter_entity) argv[2] = Nan::True();
    else                     argv[2] = Nan::False();
    if (value)               argv[3] = Nan::New(value, value_length).ToLocalChecked();
    else                     argv[3] = Nan::Null();
    if (base)                argv[4] = Nan::New(base).ToLocalChecked();
    else                     argv[4] = Nan::Null();
    if (systemId)            argv[5] = Nan::New(systemId).ToLocalChecked();
    else                     argv[5] = Nan::Null();
    if (publicId)            argv[6] = Nan::New(publicId).ToLocalChecked();
    else                     argv[6] = Nan::Null();
    if (notationName)        argv[7] = Nan::New(notationName).ToLocalChecked();
    else                     argv[7] = Nan::Null();

    parser->Emit(8, argv);
  }

  XML_Encoding *xmlEncodingInfo;

  static int UnknownEncoding(void *encodingHandlerData, const XML_Char *name, XML_Encoding *info)
  {
    Nan::HandleScope scope;
    Parser *parser = reinterpret_cast<Parser *>(encodingHandlerData);

    /* Trigger event */
    parser->xmlEncodingInfo = info;
    Local<Value> argv[2];

              argv[0] = Nan::New("unknownEncoding").ToLocalChecked();
    if (name) argv[1] = Nan::New(name).ToLocalChecked();
    else      argv[1] = Nan::Null();

    parser->Emit(2, argv);

    /* Did no event handler invoke setUnknownEncoding()? */
    if (parser->xmlEncodingInfo) {
      parser->xmlEncodingInfo = NULL;
      return XML_STATUS_ERROR;
    } else {
      return XML_STATUS_OK;
    }
  }

  /**
   * Fills xmlEncodingInfo
   */
  static NAN_METHOD(SetUnknownEncoding)
  {
    Parser *parser = Nan::ObjectWrap::Unwrap<Parser>(info.This());
    Nan::HandleScope scope;

    if (!parser->xmlEncodingInfo)
      Nan::ThrowError("setUnknownEncoding() must be synchronously invoked from an unknownEncoding event handler");

    if (info.Length() >= 1 && info[0]->IsArray()) {
      Local<Array> map = info[0].As<Array>();
      /* Copy map */
      for(int i = 0; i < 256; i++) {
        Local<Value> m = Nan::Get(map, Nan::New(i)).ToLocalChecked();
        if (m->IsInt32())
          parser->xmlEncodingInfo->map[i] = Nan::To<int32_t>(m).FromJust();
        else
          Nan::ThrowTypeError("UnknownEncoding map must consist of 256 ints");
      }
    } else
      Nan::ThrowTypeError("SetUnknownEncoding expects a map array");

    parser->xmlEncodingInfo = NULL;
    return;
  }

  void Emit(int argc, Local<Value> argv[])
  {
    Nan::HandleScope scope;

    Local<Object> handle = this->handle();
    Local<Function> emit = Nan::Get(handle, Nan::New("emit").ToLocalChecked()).ToLocalChecked().As<Function>();
    Nan::Callback emitCallback(emit);
    Nan::Call(emitCallback, argc, argv);
  }
};

extern "C" {
  static NAN_MODULE_INIT(InitAll)
  {
    Parser::Initialize(target);
  }
  //Changed the name cause I couldn't load the module with - in their names
  NODE_MODULE(node_expat, InitAll);
};

