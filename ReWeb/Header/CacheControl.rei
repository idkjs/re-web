/** See
    {{: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control}}
    for detailed explanations of the various cache options. See
    {!ReWeb.Filter.cache_control} for the intended usage of this type. */;

/** Caching options for the end user's device. */

type privately = {
  must_revalidate: option(bool),
  /** In seconds */
  max_age: option(int),
};

/** Caching options for proxies and other devices than the end user. */

type publicly = {
  no_transform: option(bool),
  proxy_revalidate: option(bool),
  /** In seconds */
  s_maxage: option(int),
};

/** Possible values of the [Cache-Control] response header. */

type t =
  | /** Don't cache at all */
    No_store
  | /** Cache but revalidate with every request */
    No_cache
  | /** Cache on end user's device only */
    Private(privately)
  | /** Cache anywhere */
    Public(privately, publicly);

/** [private_(?must_revalidate, ?max_age, ())] is a convenience function
    for creating a private cache response. */

let private: (~must_revalidate: bool=?, ~max_age: int=?, unit) => t;

/** [public(?must_revalidate, ?max_age, ?no_transform, ?proxy_revalidate, ?s_maxage, ())]
    is a convenience function for creating a public cache response. */

let public:
  (
    ~must_revalidate: bool=?,
    ~max_age: int=?,
    ~no_transform: bool=?,
    ~proxy_revalidate: bool=?,
    ~s_maxage: int=?,
    unit
  ) =>
  t;

/** [to_string(value)] converts the cache-control instructions into a
    comma-separated list of directives. */

let to_string: t => string;
