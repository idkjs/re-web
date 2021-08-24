/** You can use and override this configuration by setting the values of
    environment variables with names derived from the names in the [S]
    module type below by prefixing [REWEB__]. For example,
    [REWEB__buf_size]. You can set up your own configuration by using
    the functions below.

    Another technique to customize configuration is creating a custom
    config module. However that is primarily intended for testing and is
    covered in the manual: {!Manual.Ch06_Configuration}.

    You can also create a configuration module in your own application
    that reads values from the system environment, by using the
    (type-safe) functions below. */;

/** [string(name)] gets a value from the system environment variable
    with the name [REWEB__name]. The following functions all use the
    same name prefix and work for their specific types. */

let string = name => Sys.getenv_opt @@ "REWEB__" ++ name;

let bool = name => Option.bind(string(name), bool_of_string_opt);

/** [char(name)] gets the first character of the value of the system
    environment variable named [REWEB__name]. */

let char = name =>
  Option.bind(string(name)) @@
  (
    fun
    | "" => None
    | string => Some(String.unsafe_get(string, 0))
  );

let float = name => Option.bind(string(name), float_of_string_opt);
let int = name => Option.bind(string(name), int_of_string_opt);

/** The known ReWeb configuration settings. */
module type S = {
  module Filters: {
    /** Whether to apply a default [Content-Security-Policy] header to
        all responses--default true. */

    let csp: bool;

    /** Whether to turn on HSTS for all responses--default true. */

    let hsts: bool;
  };

  /** Whether to use HTTPS for various settings e.g. cookies, content
      security policy, etc.--default true. */

  let secure: bool;

  /** Buffer size for internal string/bigstring handling. */

  let buf_size: int;

  /** Server port--default 8080 */

  let port: int;
};

/** Default values for ReWeb configuration settings. */
module Default: S = {
  module Filters = {
    let csp = "filters__csp" |> bool |> Option.value(~default=true);
    let hsts = "filters__hsts" |> bool |> Option.value(~default=true);
  };

  let secure = "secure" |> bool |> Option.value(~default=true);

  let buf_size =
    "buf_size" |> int |> Option.value(~default=Lwt_io.default_buffer_size());

  let port = "port" |> int |> Option.value(~default=8080);
};
