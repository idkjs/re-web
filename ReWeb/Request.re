/** This interface abstracts away the Httpaf body type. */
module type BODY = {
  type t(_);

  let schedule_read:
    (
      t([ | `read]),
      ~on_eof: unit => unit,
      ~on_read: (Bigstringaf.t, ~off: int, ~len: int) => unit
    ) =>
    unit;
};

/** This interface abstracts away the Httpaf request descriptor. */
module type REQD = {
  module Body: BODY;
  type t;

  let request: t => Httpaf.Request.t;
  let request_body: t => Body.t([ | `read]);
};

module type S = {
  module Config: ReWeb__Config.S;
  module Reqd: REQD;

  /** A request. ['ctx] is the type of the request context which can be
      updated by setting a new context. It is recommended to do so in a
      way that preserves the old context (if there is one), e.g. inside
      an OCaml object with a [prev] method that points to the old
      context. */

  type t('ctx);

  /** [body(request)] gets the [request] body. There is a chance that
      the body may already have been read, in which case trying to read
      it again will error. However in a normal request pipeline as
      bodies are read by filters, that should be minimized. */

  let body: t(unit) => Piaf.Body.t;

  /** [body_form_raw(?buf_size, request)] returns the request body form
      decoded into an association list, internally using a buffer of
      size [buf_size] with a default configured by
      {!ReWeb.Config.S.buf_size}.

      @since 0.7.0 */

  let body_form_raw:
    (~buf_size: int=?, t(unit)) =>
    Lwt_result.t(list((string, list(string))), string);

  /** [body_string(?buf_size, request)] returns the request body
      converted into a string, internally using a buffer of size
      [buf_size] with a default configured by {!ReWeb.Config.S.buf_size}. */

  let body_string: (~buf_size: int=?, t(unit)) => Lwt.t(string);

  let context: t('ctx) => 'ctx;

  let cookies: t(_) => list((string, string));

  /** [header(name, request)] gets the last value corresponding to the
      given header, if present. */

  let header: (string, t(_)) => option(string);

  /** [headers(name, request)] gets all the values corresponding to the
      given header. */

  let headers: (string, t(_)) => list(string);

  /** [make(query, reqd)] returns a new request containing the given
      [query] and Httpaf [reqd]. */

  let make: (string, Reqd.t) => t(unit);

  /** [meth(request)] gets the request method ([`GET], [`POST], etc.). */

  let meth: t(_) => Httpaf.Method.t;

  /** [query(request)] gets the query string of the [request]. This is
      anything following the [?] in the request path, otherwise an empty
      string. */

  let query: t(_) => string;

  /** [set_context(ctx, request)] is an updated [request] with the given
      context [ctx]. */

  let set_context: ('ctx2, t('ctx1)) => t('ctx2);
};

module H = Httpaf;

module Make = (C: ReWeb__Config.S, R: REQD) => {
  module B = R.Body;
  module Config = C;
  module Reqd = R;

  type t('ctx) = {
    ctx: 'ctx,
    query: string,
    reqd: Reqd.t,
  };

  let body = request => {
    let request_body = Reqd.request_body(request.reqd);
    let (stream, push_to_stream) = Lwt_stream.create();
    let on_eof = () => push_to_stream(None);
    let rec on_read = (buffer, ~off, ~len) => {
      push_to_stream(
        Some({
          H.IOVec.off,
          len,
          buffer: Bigstringaf.copy(buffer, ~off, ~len),
        }),
      );
      B.schedule_read(request_body, ~on_eof, ~on_read);
    };

    B.schedule_read(request_body, ~on_eof, ~on_read);
    Piaf.Body.of_stream(stream);
  };

  let body_string = (~buf_size=Config.buf_size, request) => {
    let request_body = Reqd.request_body(request.reqd);
    let (body, set_body) = Lwt.wait();
    let buffer = Buffer.create(buf_size);
    let on_eof = () =>
      buffer |> Buffer.contents |> Lwt.wakeup_later(set_body);

    let rec on_read = (data, ~off, ~len) => {
      data |> Bigstringaf.substring(~off, ~len) |> Buffer.add_string(buffer);
      B.schedule_read(request_body, ~on_eof, ~on_read);
    };

    B.schedule_read(request_body, ~on_eof, ~on_read);
    body;
  };

  let context = ({ctx, _}) => ctx;

  let header = (name, {reqd, _}) => {
    let {H.Request.headers, _} = Reqd.request(reqd);
    H.Headers.get(headers, name);
  };

  let body_form_raw = (~buf_size=?, request) =>
    switch (header("content-type", request)) {
    | Some("application/x-www-form-urlencoded") =>
      /* TODO: implement a form query decoder that works more like what
         one would expect, i.e. understanding repeated (array) fields
         like [a[]=1&a[]=2], and erroring on invalid form data like [a]. */
      let ok = body => Ok(Uri.query_of_encoded(body));
      request |> body_string(~buf_size?) |> Lwt.map(ok);
    | _ => Lwt_result.fail("request content-type is not form")
    };

  let headers = (name, {reqd, _}) => {
    let {H.Request.headers, _} = Reqd.request(reqd);
    H.Headers.get_multi(headers, name);
  };

  let parse_cookie = string =>
    switch (string |> String.trim |> String.split_on_char('=')) {
    | [name, value] => Some((name, value))
    | _ => None
    };

  let cookie_of_header = value =>
    value |> String.split_on_char(';') |> List.filter_map(parse_cookie);

  let cookies = request =>
    request
    |> headers("cookie")
    |> List.map(cookie_of_header)
    |> List.flatten;

  let make = (query, reqd) => {ctx: (), query, reqd};
  let meth = ({reqd, _}) => Reqd.request(reqd).H.Request.meth;
  let query = ({query, _}) => query;
  let set_context = (ctx, request) => {...request, ctx};
};
