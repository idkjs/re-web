/** For the response functions below, [status] defaults to [`OK] unless
    otherwise noted, and the [cookies] parameter is converted into
    response headers and merged with the [headers] parameter. There is
    therefore a chance of duplication which you will have to watch out
    for. */;

module Header = ReWeb__Header;

/** a header [X-Client-Id: 1] is represented as:
    [[("x-client-id", "1")]]. */

type headers = list((string, string));

/** See {{: https://b0-system.github.io/odig/doc@odoc.default/httpaf/Httpaf/Status/index.html} Httpaf.Status}
    for valid statuses. */

type status = Httpaf.Status.t;

type http = [ | `HTTP(Httpaf.Response.t, Piaf.Body.t)];

/** Possible issues with pulling a message from the incoming messages
    stream. */

type pull_error = [
  | /** The incoming message stream is empty. */
    `Empty
  | /** Could not get a message from the stream within the given timeout. */
    `Timeout
  | /** Connection was closed by the client. {b Warning:} you must exit
      the WebSocket handler as soon as possible when this happens.
      Otherwise, you will be in an infinite loop waiting for messages
      that will never arrive. */
    `Connection_close
];

/** [pull(timeout_s)] asynchronously gets the next message from the
    WebSocket if there is any, with a timeout in seconds of [timeout_s].
    If it doesn't time out it returns [Some string], otherwise [None]. */

type pull = float => Lwt_result.t(string, pull_error);

/** [push(response)] pushes the string [response] to the WebSocket
    client. */

type push = string => unit;

/** [handler(pull, push)] is an asynchronous callback that manages the
    WS from the server side. The WS will shut down from the server side
    as soon as [handler] exits, so if you want to keep it open you need
    to make it call itself recursively. Because the call will be
    tail-recursive, OCaml's tail-call elimination takes care of stack
    memory use. */

type handler = (pull, push) => Lwt.t(unit);

/** A WebSocket response. */

type websocket = [ | `WebSocket(option(Httpaf.Headers.t), handler)];

/** Response type, can be an HTTP or a WebSocket response. Many of the
    functions below work with either HTTP or WebSocket responses, and
    some with only one or the other. This is enforced at the type level. */

type t('resp) = [> http | websocket] as 'resp;

/** [add_cookie(cookie, response)] returns a response with a cookie
    [cookie] added to the original [response]. */

let add_cookie: (Header.SetCookie.t, [< http | websocket]) => t(_);

/** [add_cookies(cookies, response)] returns a response with the
    [cookies] added to the original [response]. */

let add_cookies: (list(Header.SetCookie.t), [< http | websocket]) => t(_);

/** [add_header(?replace, ~name, ~value, response)] returns a response
    with a header [name] with value [value] added to the original
    [response]. If the response already contains the [header], then it
    is replaced only if [replace] is [true], which is the default. */

let add_header:
  (~replace: bool=?, ~name: string, ~value: string, [< http | websocket]) =>
  t(_);

/** [add_headers(headers, response)] returns a response with the
    [headers] added to the end of the original [response]'s header list. */

let add_headers: (headers, [< http | websocket]) => t(_);

/** [add_headers_multi(headers_multi, response)] returns a response with
    [headers_multi] added to the end of the original [response]'s header
    list. */

let add_headers_multi:
  (list((string, list(string))), [< http | websocket]) => t(_);

let body: [< http] => Piaf.Body.t;

let cookies: [< http | websocket] => list(Header.SetCookie.t);

/** [header(name, request)] gets the last value corresponding to the
    given header, if present. */

let header: (string, [< http | websocket]) => option(string);

/** [headers(name, request)] gets all the values corresponding to the
    given header. */

let headers: (string, [< http | websocket]) => list(string);

let of_binary:
  (
    ~status: status=?,
    ~content_type: string=?,
    ~headers: headers=?,
    ~cookies: list(Header.SetCookie.t)=?,
    string
  ) =>
  [> http];

/** [of_file(?status, ?content_type, ?headers, ?cookies, file_name)]
    responds with the contents of [file_name], which is a relative or
    absolute path, with HTTP response code [status] and content-type
    header [content_type].

    If the file is not found, responds with a 404 Not Found status and
    an appropriate message.

    {i Warning} this function maps the entire file into memory. Don't
    use it for files larger than memory. */

let of_file:
  (
    ~status: status=?,
    ~content_type: string=?,
    ~headers: headers=?,
    ~cookies: list(Header.SetCookie.t)=?,
    string
  ) =>
  Lwt.t([> http]);

let of_html:
  (
    ~status: status=?,
    ~headers: headers=?,
    ~cookies: list(Header.SetCookie.t)=?,
    string
  ) =>
  [> http];

/** [of_http(~status, ~headers, body)] responds with an HTTP response
    composed of [status], [headers], and [body]. */

let of_http:
  (~status: status, ~headers: list((string, string)), Piaf.Body.t) =>
  [> http];

let of_json:
  (
    ~status: status=?,
    ~headers: headers=?,
    ~cookies: list(Header.SetCookie.t)=?,
    Yojson.Safe.t
  ) =>
  [> http];

/** [of_redirect(?content_type, ?body, location)] responds with an HTTP
    redirect response to the new [location], with an optional
    [content_type] (defaulting to [text/plain]) and [body] (defaulting
    to an empty body). */

let of_redirect:
  (~content_type: string=?, ~body: string=?, string) => [> http];

/** [of_status(?content_type, ?headers, ?cookies, ?message, status)]
    responds with a standard boilerplate response message based on the
    [content_type] and [status]. [content_type] defaults to [`text]. The
    boilerplate message can be overridden by [message] if present. */

let of_status:
  (
    ~content_type: [ | `text | `html]=?,
    ~headers: headers=?,
    ~cookies: list(Header.SetCookie.t)=?,
    ~message: string=?,
    status
  ) =>
  [> http];

let of_text:
  (
    ~status: status=?,
    ~headers: headers=?,
    ~cookies: list(Header.SetCookie.t)=?,
    string
  ) =>
  [> http];

/** [of_view(?status, ?content_type, ?headers, ?cookies, view)] responds
    with a rendered body as per the [view] function. The [view] is a
    function that takes a 'printer' function ([string -> unit]) as a
    parameter and 'prints' (i.e. renders piecemeal) strings to it. These
    strings are pushed out as they are rendered.

    The difference from [of_html], [of_binary], and the other functions
    is that those hold the entire response in memory before sending it
    to the client, while [of_view] holds only each piece of the response
    as it is streamed out. */

let of_view:
  (
    ~status: status=?,
    ~content_type: string=?,
    ~headers: headers=?,
    ~cookies: list(Header.SetCookie.t)=?,
    (string => unit) => unit
  ) =>
  [> http];

/** [of_websocket(?headers, handler)] is an open WebSocket response.
    Optionally you can pass along extra [headers] which will be sent to
    the client when opening the WS.

    {i Warning} it can be a little tricky to write a completely
    asynchronous WebSocket handler correctly. Be sure to read the
    reference documentation above, and the manual, carefully.

    {i Note} OCaml strings are un-encoded byte arrays, and ReWeb treats
    all incoming and outgoing WebSocket data as such--even if the client
    is sending UTF-8 encoded text format (see
    {{: https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers#Format}}).
    It's up to you as the WebSocket handler writer to encode/decode them
    as necessary. */

let of_websocket: (~headers: headers=?, handler) => [> websocket];

let status: [< http] => status;
let status_code: [< http] => int;
