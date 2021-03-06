#ifndef __PROCESS_PROTOBUF_HPP__
#define __PROCESS_PROTOBUF_HPP__

#include <glog/logging.h>

#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>

#include <set>
#include <vector>

#include <tr1/functional>
#include <tr1/unordered_map>

#include <process/dispatch.hpp>
#include <process/process.hpp>


// Provides an implementation of process::post that for a protobuf.
namespace process {

inline void post(const process::UPID& to,
                 const google::protobuf::Message& message)
{
  std::string data;
  message.SerializeToString(&data);
  post(to, message.GetTypeName(), data.data(), data.size());
}


inline void post(const process::UPID& from,
                 const process::UPID& to,
                 const google::protobuf::Message& message)
{
  std::string data;
  message.SerializeToString(&data);
  post(from, to, message.GetTypeName(), data.data(), data.size());
}

} // namespace process {


// The rest of this file provides libprocess "support" for using
// protocol buffers. In particular, this file defines a subclass of
// Process (ProtobufProcess) that allows you to install protocol
// buffer handlers in addition to normal message and HTTP
// handlers. Note that this header file assumes you will be linking
// against BOTH libprotobuf and libglog.

namespace google { namespace protobuf {

// Type conversions helpful for changing between protocol buffer types
// and standard C++ types (for parameters).
template <typename T>
const T& convert(const T& t)
{
  return t;
}


template <typename T>
std::vector<T> convert(const google::protobuf::RepeatedPtrField<T>& items)
{
  std::vector<T> result;
  for (int i = 0; i < items.size(); i++) {
    result.push_back(items.Get(i));
  }

  return result;
}

}} // namespace google { namespace protobuf {


template <typename T>
class ProtobufProcess : public process::Process<T>
{
public:
  virtual ~ProtobufProcess() {}

protected:
  virtual void visit(const process::MessageEvent& event)
  {
    if (protobufHandlers.count(event.message->name) > 0) {
      from = event.message->from; // For 'reply'.
      protobufHandlers[event.message->name](event.message->body);
      from = process::UPID();
    } else {
      process::Process<T>::visit(event);
    }
  }

  void send(const process::UPID& to,
            const google::protobuf::Message& message)
  {
    std::string data;
    message.SerializeToString(&data);
    process::Process<T>::send(to, message.GetTypeName(),
                              data.data(), data.size());
  }

  using process::Process<T>::send;

  void reply(const google::protobuf::Message& message)
  {
    CHECK(from) << "Attempting to reply without a sender";
    std::string data;
    message.SerializeToString(&data);
    send(from, message);
  }

  template <typename M>
  void install(void (T::*method)(const M&))
  {
    google::protobuf::Message* m = new M();
    T* t = static_cast<T*>(this);
    protobufHandlers[m->GetTypeName()] =
      std::tr1::bind(&handlerM<M>,
                     t, method,
                     std::tr1::placeholders::_1);
    delete m;
  }

  template <typename M>
  void install(void (T::*method)())
  {
    google::protobuf::Message* m = new M();
    T* t = static_cast<T*>(this);
    protobufHandlers[m->GetTypeName()] =
      std::tr1::bind(&handler0,
                     t, method,
                     std::tr1::placeholders::_1);
    delete m;
  }

  template <typename M,
            typename P1, typename P1C>
  void install(void (T::*method)(P1C),
                              P1 (M::*param1)() const)
  {
    google::protobuf::Message* m = new M();
    T* t = static_cast<T*>(this);
    protobufHandlers[m->GetTypeName()] =
      std::tr1::bind(&handler1<M, P1, P1C>,
                     t, method, param1,
                     std::tr1::placeholders::_1);
    delete m;
  }

  template <typename M,
            typename P1, typename P1C,
            typename P2, typename P2C>
  void install(void (T::*method)(P1C, P2C),
                              P1 (M::*p1)() const,
                              P2 (M::*p2)() const)
  {
    google::protobuf::Message* m = new M();
    T* t = static_cast<T*>(this);
    protobufHandlers[m->GetTypeName()] =
      std::tr1::bind(&handler2<M, P1, P1C, P2, P2C>,
                     t, method, p1, p2,
                     std::tr1::placeholders::_1);
    delete m;
  }

  template <typename M,
            typename P1, typename P1C,
            typename P2, typename P2C,
            typename P3, typename P3C>
  void install(void (T::*method)(P1C, P2C, P3C),
                              P1 (M::*p1)() const,
                              P2 (M::*p2)() const,
                              P3 (M::*p3)() const)
  {
    google::protobuf::Message* m = new M();
    T* t = static_cast<T*>(this);
    protobufHandlers[m->GetTypeName()] =
      std::tr1::bind(&handler3<M, P1, P1C, P2, P2C, P3, P3C>,
                     t, method, p1, p2, p3,
                     std::tr1::placeholders::_1);
    delete m;
  }

  template <typename M,
            typename P1, typename P1C,
            typename P2, typename P2C,
            typename P3, typename P3C,
            typename P4, typename P4C>
  void install(void (T::*method)(P1C, P2C, P3C, P4C),
                              P1 (M::*p1)() const,
                              P2 (M::*p2)() const,
                              P3 (M::*p3)() const,
                              P4 (M::*p4)() const)
  {
    google::protobuf::Message* m = new M();
    T* t = static_cast<T*>(this);
    protobufHandlers[m->GetTypeName()] =
      std::tr1::bind(&handler4<M, P1, P1C, P2, P2C, P3, P3C, P4, P4C>,
                     t, method, p1, p2, p3, p4,
                     std::tr1::placeholders::_1);
    delete m;
  }

  template <typename M,
            typename P1, typename P1C,
            typename P2, typename P2C,
            typename P3, typename P3C,
            typename P4, typename P4C,
            typename P5, typename P5C>
  void install(void (T::*method)(P1C, P2C, P3C, P4C, P5C),
                              P1 (M::*p1)() const,
                              P2 (M::*p2)() const,
                              P3 (M::*p3)() const,
                              P4 (M::*p4)() const,
                              P5 (M::*p5)() const)
  {
    google::protobuf::Message* m = new M();
    T* t = static_cast<T*>(this);
    protobufHandlers[m->GetTypeName()] =
      std::tr1::bind(&handler5<M, P1, P1C, P2, P2C, P3, P3C, P4, P4C, P5, P5C>,
                     t, method, p1, p2, p3, p4, p5,
                     std::tr1::placeholders::_1);
    delete m;
  }

  using process::Process<T>::install;

  process::UPID from; // Sender of "current" message, accessible by subclasses.

private:
  template <typename M>
  static void handlerM(T* t, void (T::*method)(const M&),
                       const std::string& data)
  {
    M m;
    m.ParseFromString(data);
    if (m.IsInitialized()) {
      (t->*method)(m);
    } else {
      LOG(WARNING) << "Initialization errors: "
                   << m.InitializationErrorString();
    }
  }

  static void handler0(T* t, void (T::*method)(),
                       const std::string& data)
  {
    (t->*method)();
  }

  template <typename M,
            typename P1, typename P1C>
  static void handler1(T* t, void (T::*method)(P1C),
                       P1 (M::*p1)() const,
                       const std::string& data)
  {
    M m;
    m.ParseFromString(data);
    if (m.IsInitialized()) {
      (t->*method)(google::protobuf::convert((&m->*p1)()));
    } else {
      LOG(WARNING) << "Initialization errors: "
                   << m.InitializationErrorString();
    }
  }

  template <typename M,
            typename P1, typename P1C,
            typename P2, typename P2C>
  static void handler2(T* t, void (T::*method)(P1C, P2C),
                       P1 (M::*p1)() const,
                       P2 (M::*p2)() const,
                       const std::string& data)
  {
    M m;
    m.ParseFromString(data);
    if (m.IsInitialized()) {
      (t->*method)(google::protobuf::convert((&m->*p1)()),
                   google::protobuf::convert((&m->*p2)()));
    } else {
      LOG(WARNING) << "Initialization errors: "
                   << m.InitializationErrorString();
    }
  }

  template <typename M,
            typename P1, typename P1C,
            typename P2, typename P2C,
            typename P3, typename P3C>
  static void handler3(T* t, void (T::*method)(P1C, P2C, P3C),
                       P1 (M::*p1)() const,
                       P2 (M::*p2)() const,
                       P3 (M::*p3)() const,
                       const std::string& data)
  {
    M m;
    m.ParseFromString(data);
    if (m.IsInitialized()) {
      (t->*method)(google::protobuf::convert((&m->*p1)()),
                   google::protobuf::convert((&m->*p2)()),
                   google::protobuf::convert((&m->*p3)()));
    } else {
      LOG(WARNING) << "Initialization errors: "
                   << m.InitializationErrorString();
    }
  }

  template <typename M,
            typename P1, typename P1C,
            typename P2, typename P2C,
            typename P3, typename P3C,
            typename P4, typename P4C>
  static void handler4(T* t, void (T::*method)(P1C, P2C, P3C, P4C),
                       P1 (M::*p1)() const,
                       P2 (M::*p2)() const,
                       P3 (M::*p3)() const,
                       P4 (M::*p4)() const,
                       const std::string& data)
  {
    M m;
    m.ParseFromString(data);
    if (m.IsInitialized()) {
      (t->*method)(google::protobuf::convert((&m->*p1)()),
                   google::protobuf::convert((&m->*p2)()),
                   google::protobuf::convert((&m->*p3)()),
                   google::protobuf::convert((&m->*p4)()));
    } else {
      LOG(WARNING) << "Initialization errors: "
                   << m.InitializationErrorString();
    }
  }

  template <typename M,
            typename P1, typename P1C,
            typename P2, typename P2C,
            typename P3, typename P3C,
            typename P4, typename P4C,
            typename P5, typename P5C>
  static void handler5(T* t, void (T::*method)(P1C, P2C, P3C, P4C, P5C),
                       P1 (M::*p1)() const,
                       P2 (M::*p2)() const,
                       P3 (M::*p3)() const,
                       P4 (M::*p4)() const,
                       P5 (M::*p5)() const,
                       const std::string& data)
  {
    M m;
    m.ParseFromString(data);
    if (m.IsInitialized()) {
      (t->*method)(google::protobuf::convert((&m->*p1)()),
                   google::protobuf::convert((&m->*p2)()),
                   google::protobuf::convert((&m->*p3)()),
                   google::protobuf::convert((&m->*p4)()),
                   google::protobuf::convert((&m->*p5)()));
    } else {
      LOG(WARNING) << "Initialization errors: "
                   << m.InitializationErrorString();
    }
  }

  typedef std::tr1::function<void(const std::string&)> handler;
  std::tr1::unordered_map<std::string, handler> protobufHandlers;
};


// Implements a process for sending protobuf "requests" to a process
// and waiting for a protobuf "response", but uses futures so that
// this can be done without needing to implement a process.
template <typename Req, typename Res>
class ReqResProcess : public ProtobufProcess<ReqResProcess<Req, Res> >
{
public:
  ReqResProcess(const process::UPID& _pid, const Req& _req)
    : pid(_pid), req(_req)
  {
    ProtobufProcess<ReqResProcess<Req, Res> >::template
      install<Res>(&ReqResProcess<Req, Res>::response);
  }

  process::Future<Res> run()
  {
    // Terminate this process if no one cares about the response
    // (note, we need to disambiguate the process::terminate).
    void (*terminate)(const process::UPID&, bool) = &process::terminate;
    promise.future().onDiscarded(
        std::tr1::bind(terminate, process::ProcessBase::self(), true));

    ProtobufProcess<ReqResProcess<Req, Res> >::send(pid, req);

    return promise.future();
  }

private:
  void response(const Res& res)
  {
    promise.set(res);
    process::terminate(process::ProcessBase::self());
  }

  const process::UPID pid;
  const Req req;
  process::Promise<Res> promise;
};


// Allows you to describe request/response protocols and then use
// those for sending requests and getting back responses.
template <typename Req, typename Res>
struct Protocol
{
  process::Future<Res> operator () (
      const process::UPID& pid,
      const Req& req) const
  {
    // Help debugging by adding some "type constraints".
    { Req* req = NULL; google::protobuf::Message* m = req; (void)m; }
    { Res* res = NULL; google::protobuf::Message* m = res; (void)m; }

    ReqResProcess<Req, Res>* process = new ReqResProcess<Req, Res>(pid, req);
    process::spawn(process, true);
    return process::dispatch(process, &ReqResProcess<Req, Res>::run);
  }
};

#endif // __PROCESS_PROTOBUF_HPP__
