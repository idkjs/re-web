/** The request functions below return a response of type
    [([> Response.http], string) Lwt_result.t]. This is a promise
    containing a [result] of either [Ok response] where [response] is an
    HTTP response, or a [string] containing an error message. */;

/** See the Piaf module documentation for more information on these
    options. */

type config =
  Piaf.Config.t = {
    follow_redirects: bool,
    max_redirects: int,
    allow_insecure: bool,
    max_http_version: Piaf.Versions.HTTP.t,
    h2c_upgrade: bool,
    http2_prior_knowledge: bool,
    cacert: option(string),
    capath: option(string),
    min_tls_version: Piaf.Versions.TLS.t,
    max_tls_version: Piaf.Versions.TLS.t,
    tcp_nodelay: bool,
    connect_timeout: float,
    buffer_size: int,
    body_buffer_size: int,
    enable_http2_server_push: bool,
  };

type headers = list((string, string));

/** Use this [config] value to override the default config in client
    requests. */

let config: config;

/** Make requests with a one-shot i.e. stateless client. */

module New: {
  /** The type of request functions which send a request body. */

  type request_body('resp) =
    (~config: config=?, ~headers: headers=?, ~body: Piaf.Body.t=?, string) =>
    Lwt_result.t([> Response.http] as 'resp, string);

  /** The type of request functions which don't send a request body. */

  type request_nobody('resp) =
    (~config: config=?, ~headers: headers=?, string) =>
    Lwt_result.t([> Response.http] as 'resp, string);

  let delete: request_body([> Response.http]);
  let get: request_nobody([> Response.http]);
  let head: request_nobody([> Response.http]);
  let patch: request_body([> Response.http]);
  let post: request_body([> Response.http]);
  let put: request_body([> Response.http]);

  /** Make a request not covered by the above functions. */

  let request:
    (
      ~config: config=?,
      ~headers: headers=?,
      ~body: Piaf.Body.t=?,
      ~meth: Piaf.Method.t,
      string
    ) =>
    Lwt_result.t([> Response.http], string);
};
