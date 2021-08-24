/** Convenience module for creating a
    {{: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Security-Policy} Content-Security-Policy}
    header. {i Note} that to set up a reporting endpoint properly, you
    will need to use {!ReWeb.Filter.csp}. */;

type src =
  | Host(string)
  | Scheme([ | `HTTP | `HTTPS | `Data | `Mediastream | `Blob | `Filesystem])
  | Self
  | Unsafe_eval
  | Unsafe_hashes
  | Unsafe_inline
  | None
  | Nonce(string)
  | Hash([ | `SHA256 | `SHA384 | `SHA512], string);

type src_list = option(list(src));

/** CSP header value data model. */

type t =
  pri {
    child_src: src_list,
    connect_src: src_list,
    default_src: list(src),
    font_src: src_list,
    frame_src: src_list,
    img_src: src_list,
    manifest_src: src_list,
    media_src: src_list,
    object_src: src_list,
    prefetch_src: src_list,
    script_src: src_list,
    script_src_elem: src_list,
    script_src_attr: src_list,
    style_src: src_list,
    style_src_elem: src_list,
    style_src_attr: src_list,
    worker_src: src_list,
    base_uri: src_list,
    plugin_types: option(list(string)),
    form_action: src_list,
    navigate_to: src_list,
    report_to: option(list(string)),
    block_all_mixed_content: option(bool),
  };

let has_report_to: option(list(string)) => bool;

/** [make(..., default_src)] is a content security policy consisting of
    the given options. */

let make:
  (
    ~child_src: list(src)=?,
    ~connect_src: list(src)=?,
    ~font_src: list(src)=?,
    ~frame_src: list(src)=?,
    ~img_src: list(src)=?,
    ~manifest_src: list(src)=?,
    ~media_src: list(src)=?,
    ~object_src: list(src)=?,
    ~prefetch_src: list(src)=?,
    ~script_src: list(src)=?,
    ~script_src_elem: list(src)=?,
    ~script_src_attr: list(src)=?,
    ~style_src: list(src)=?,
    ~style_src_elem: list(src)=?,
    ~style_src_attr: list(src)=?,
    ~worker_src: list(src)=?,
    ~base_uri: list(src)=?,
    ~plugin_types: list(string)=?,
    ~form_action: list(src)=?,
    ~navigate_to: list(src)=?,
    ~report_to: list(string)=?,
    ~block_all_mixed_content: bool=?,
    list(src)
  ) =>
  t;

/** [report_to_string(directives)] is a valid [Report-To] header using
    the [directives]. */

let report_to_header: t => (string, string);

/** [to_header(?report_only, directives)] is a either a
    [content-security-policy] header (if [report_only] is [false] which
    is the default), or a [content-security-policy-report-only] header
    if [report_only] is [true].

    If [directives] just contains empty lists, [to_header] will output
    the [default-src 'self'] directive under the assumption that you
    want some protection since you're trying to use CSP.

    If [directives] contains a non-empty [report_to], will output both
    [report-uri] and [report-to] directives to try to target browsers
    that support either. */

let to_header: (~report_only: bool=?, t) => (string, string);
