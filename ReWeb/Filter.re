module Header = ReWeb__Header;

module type S = {
  module Config: ReWeb__Config.S;
  module Service: Service.S;

  /** A filter transforms a service. It can change the request (usually
      by changing the request context) or the response (by actually
      running the service and then modifying its response).

      Filters can be composed using function composition. */

  type t('ctx1, 'ctx2, 'resp) =
    Service.t('ctx2, 'resp) => Service.t('ctx1, 'resp);

  /** [basic_auth] decodes and stores the login credentials sent with
      the [Authorization] header or returns a 401 Unauthorized error if
      there is none. */

  let basic_auth:
    t(
      'ctx1,
      {
        .
        username: string,
        password: string,
        prev: 'ctx1,
      },
      Response.t(_),
    );

  /** [bearer_auth] stores the bearer token sent with the
      [Authorization] header or returns a 401 Unauthorized error if
      there is none. */

  let bearer_auth:
    t(
      'ctx1,
      {
        .
        bearer_token: string,
        prev: 'ctx1,
      },
      Response.t(_),
    );

  /** [body_form(typ)] is a filter that decodes a web form in the
      request body and puts it inside the request for the next service.
      The decoding is done as specified by the form definition [typ]. If
      the form fails to decode, it short-circuits and returns a 400 Bad
      Request. */

  let body_form: Form.t('ctor, 'ty) => t(unit, 'ty, [> Response.http]);

  /** [body_json] is a filter that transforms a 'root' service (i.e. one
      with [unit] context) into a service with a context containing the
      request body. If the request body fails to parse as valid JSON, it
      short-circuits and returns a 400 Bad Request. */

  let body_json: t(unit, Yojson.Safe.t, [> Response.http]);

  /** [body_json_decode(decoder)] is a filter that transforms a service
      with a parsed JSON structure in its context, to a service with a
      decoded value of type ['ty] in its context. If the request body
      fails to decode with [decoder], the filter short-circuits and
      returns a 400 Bad Request. */

  let body_json_decode:
    (Yojson.Safe.t => result('ty, string)) =>
    t(Yojson.Safe.t, 'ty, [> Response.http]);

  /** [body_string] is a filter that transforms a 'root' service into a
      service whose context contains the request body as a single
      string. */

  let body_string: t(unit, string, [> Response.http]);

  /** [cache_control(policy)] is a filter that applies the caching
      [policy] policy to the HTTP response. */

  let cache_control:
    Header.CacheControl.t =>
    t('ctx, 'ctx, [ Response.http | Response.websocket]);

  /** [cors(origin)] adds an
      {{: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Access-Control-Allow-Origin} Access-Control-Allow-Origin}
      header with the given [origin].

      {i Note} that it's upto you to pass in a well-formed origin
      string. The [Header.AccessControlAllowOrigin] module does not
      validate the origin string. */

  let cors:
    Header.AccessControlAllowOrigin.t =>
    t('ctx, 'ctx, [ Response.http | Response.websocket]);

  /** [csp(directives)] is a filter that applies the
      [Content-Security-Policy] header [directives] to the response. */

  let csp:
    Header.ContentSecurityPolicy.t =>
    t('ctx, 'ctx, [ Response.http | Response.websocket]);

  /** [hsts(value)] is a filter that applies the HTTP Strict Transport
      Security header to the response. */

  let hsts:
    Header.StrictTransportSecurity.t =>
    t('ctx, 'ctx, [ Response.http | Response.websocket]);

  /** [multipart_form(~typ, path)] is a filter that decodes multipart
      form data. [typ] must be provided but if you don't actually have
      any other fields in the form you can use [Form.empty] to decode
      into an 'empty' (unit) value.

      [path(~filename, name)] is used to get the filesystem absolute
      path to save the given [filename] with corresponding form field
      [name]. Note that:

      - The file will be overwritten if it already exists on disk
      - [filename] is the basename, not the full path
      - The filter will short-circuit with a 401 Unauthorized error
        response if any of the files can't be opened for writing.

      This callback gives you a chance to sanitize incoming filenames
      before storing the files on disk. */

  let multipart_form:
    (~typ: Form.t('ctor, 'ty), (~filename: string, string) => string) =>
    t(unit, 'ty, [> Response.http]);

  /** [query_form(typ)] is a filter that decodes the request query (the
      part after the [?] in the endpoint) into a value of type ['ty] and
      stores it in the request context for the next service. The
      decoding and failure works in the same way as for [body_form]. */

  let query_form:
    Form.t('ctor, 'ty) =>
    t(
      'ctx,
      {
        .
        query: 'ty,
        prev: 'ctx,
      },
      Response.t(_),
    );
};

module H = Httpaf;

module Make =
       (R: Request.S)

         : (
           S with
             type Service.Request.Reqd.t = R.Reqd.t and
             type Service.Request.t('ctx) = R.t('ctx)
       ) => {
  module Service = Service.Make(R);
  module Config = Service.Config;

  type t('ctx1, 'ctx2, 'resp) =
    Service.t('ctx2, 'resp) => Service.t('ctx1, 'resp);

  let get_auth = request => {
    open Let.Option;
    let* value = R.header("Authorization", request);
    switch (String.split_on_char(' ', value)) {
    | [typ, credentials] => Some((typ, credentials))
    | _ => None
    };
  };

  let bad_request = message =>
    `Bad_request |> Response.of_status(~message) |> Lwt.return;

  let unauthorized = `Unauthorized |> Response.of_status |> Lwt.return;

  let basic_auth = (next, request) =>
    switch (get_auth(request)) {
    | Some(("Basic", credentials)) =>
      switch (Base64.decode_exn(credentials)) {
      | credentials =>
        switch (String.split_on_char(':', credentials)) {
        | [username, password] =>
          let ctx = {
            as _;
            pub username = username;
            pub password = password;
            pub prev = R.context(request)
          };

          request |> R.set_context(ctx) |> next;
        | _ => unauthorized
        }
      | exception _ => unauthorized
      }
    | _ => unauthorized
    };

  let bearer_auth = (next, request) =>
    switch (get_auth(request)) {
    | Some(("Bearer", token)) =>
      let ctx = {
        as _;
        pub bearer_token = token;
        pub prev = R.context(request)
      };

      request |> R.set_context(ctx) |> next;
    | _ => unauthorized
    };

  let body_json_bad = string =>
    bad_request("ReWeb.Filter.body_json: " ++ string);

  let body_json = (next, request) => {
    let body = R.body(request);
    open Lwt.Syntax;
    let* body_string = Piaf.Body.to_string(body);
    switch (body_string) {
    | Ok(body_string) =>
      switch (Yojson.Safe.from_string(body_string)) {
      | ctx => request |> R.set_context(ctx) |> next
      | exception (Yojson.Json_error(string)) => body_json_bad(string)
      }
    | Error(_) => body_json_bad("could not read request body")
    };
  };

  let body_json_decode = (decoder, next, request) =>
    switch (request |> R.context |> decoder) {
    | Ok(ctx) => request |> R.set_context(ctx) |> next
    | Error(string) => bad_request(string)
    };

  let body_string = (next, request) => {
    open Lwt.Syntax;
    let* ctx = R.body_string(request);
    request |> R.set_context(ctx) |> next;
  };

  let body_form = (typ, next, request) => {
    open Lwt.Syntax;
    let* raw = R.body_form_raw(request);
    switch (raw) {
    | Ok(raw) =>
      switch (Form.decode(typ, raw)) {
      | Ok(ctx) => request |> R.set_context(ctx) |> next
      | Error(string) => bad_request(string)
      }
    | Error(string) => bad_request("ReWeb.Filter.form: " ++ string)
    };
  };

  let cache_control = (policy, next, request) =>
    request
    |> next
    |> Lwt.map @@
    Response.add_header(
      ~name="cache-control",
      ~value=Header.CacheControl.to_string(policy),
    );

  let cors = (origin, next, request) =>
    request
    |> next
    |> Lwt.map @@
    Response.add_headers([
      Header.AccessControlAllowOrigin.to_header(origin),
      ("vary", "Origin"),
    ]);

  let csp = (directives, next, request) => {
    open Header.ContentSecurityPolicy;
    let headers = [to_header(directives)];
    let headers =
      if (has_report_to(directives.report_to)) {
        [report_to_header(directives), ...headers];
      } else {
        headers;
      };

    request |> next |> Lwt.map @@ Response.add_headers(headers);
  };

  let hsts = (value, next, request) => {
    let (name, value) = Header.StrictTransportSecurity.to_header(value);
    request |> next |> Lwt.map @@ Response.add_header(~name, ~value);
  };

  let multipart_ct_length = 30;

  /* Complex because we need to keep track of files being uploaded */
  let multipart_form = (~typ, path, next, request) =>
    switch (R.meth(request), R.header("content-type", request)) {
    | (`POST, Some(content_type))
        when
          String.length(content_type) > multipart_ct_length
          && String.sub(content_type, 0, multipart_ct_length)
          == "multipart/form-data; boundary=" =>
      let (stream, promise) = request |> R.body |> Piaf.Body.to_string_stream;
      let files = Hashtbl.create(~random=true, 5);
      open Lwt.Syntax;
      let close = (_, file, prev) => {
        let* () = prev;
        Lwt_unix.close(file);
      };

      let cleanup = () => Hashtbl.fold(close, files, Lwt.return_unit);
      let callback = (~name, ~filename, string) => {
        let filename = path(~filename=Filename.basename(filename), name);

        let write = file =>
          string
          |> String.length
          |> Lwt_unix.write_string(file, string, 0)
          |> Lwt.map(ignore);

        switch (Hashtbl.find_opt(files, filename)) {
        | Some(file) => write(file)
        | None =>
          let* file =
            Lwt_unix.openfile(
              filename,
              Unix.[O_CREAT, O_TRUNC, O_WRONLY, O_NONBLOCK],
              0o600,
            );
          Hashtbl.add(files, filename, file);
          write(file);
        };
      };

      let f = () =>
        Multipart_form_data.parse(~stream, ~content_type, ~callback);

      let g = fields => {
        let* () = cleanup();
        let* result = promise;
        switch (result) {
        | Ok () =>
          let fields = List.map(((k, v)) => (k, [v]), fields);
          switch (Form.decode(typ, fields)) {
          | Ok(ctx) => request |> R.set_context(ctx) |> next
          | Error(string) => bad_request(string)
          };
        | Error(_) =>
          bad_request(
            "ReWeb.Filter.multipart_form: could not read request body",
          )
        };
      };

      Lwt.try_bind(f, g) @@
      (
        exn => {
          let* () = cleanup();
          [@warning "-4"]
          (
            switch (exn) {
            | [@implicit_arity] Unix.Unix_error(Unix.EPERM, _, _) => unauthorized
            | _ =>
              let message =
                exn
                |> Printexc.to_string
                |> (++)("ReWeb.Filter.multpart_form: ");

              `Internal_server_error
              |> Response.of_status(~message)
              |> Lwt.return;
            }
          );
        }
      );
    | _ =>
      bad_request("ReWeb.Filter.multipart_form: request is not well-formed")
    };

  let query_form = (typ, next, request) =>
    switch (request |> R.query |> Form.decoder(typ)) {
    | Ok(obj) =>
      let ctx = {as _; pub query = obj; pub prev = R.context(request)};

      request |> R.set_context(ctx) |> next;
    | Error(string) => bad_request(string)
    };
};
