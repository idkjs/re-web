/** Convenience to create the
    {{: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Strict-Transport-Security} HSTS}
    header. */;

type t =
  pri {
    /** In seconds */
    max_age: int,
    include_subdomains: bool,
    preload: bool,
  };

/** [make(?include_subdomains, ?preload, max_age)] represents an HSTS
    header with the given options. */

let make: (~include_subdomains: bool=?, ~preload: bool=?, int) => t;

let to_header: t => (string, string);
