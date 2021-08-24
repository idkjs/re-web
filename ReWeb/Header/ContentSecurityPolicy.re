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

type t = {
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

let make =
    (
      ~child_src=?,
      ~connect_src=?,
      ~font_src=?,
      ~frame_src=?,
      ~img_src=?,
      ~manifest_src=?,
      ~media_src=?,
      ~object_src=?,
      ~prefetch_src=?,
      ~script_src=?,
      ~script_src_elem=?,
      ~script_src_attr=?,
      ~style_src=?,
      ~style_src_elem=?,
      ~style_src_attr=?,
      ~worker_src=?,
      ~base_uri=?,
      ~plugin_types=?,
      ~form_action=?,
      ~navigate_to=?,
      ~report_to=?,
      ~block_all_mixed_content=?,
      default_src,
    ) => {
  child_src,
  connect_src,
  default_src:
    switch (default_src) {
    | [] => [Self]
    | _ => default_src
    },
  font_src,
  frame_src,
  img_src,
  manifest_src,
  media_src,
  object_src,
  prefetch_src,
  script_src,
  script_src_elem,
  script_src_attr,
  style_src,
  style_src_elem,
  style_src_attr,
  worker_src,
  base_uri,
  plugin_types,
  form_action,
  navigate_to,
  report_to,
  block_all_mixed_content,
};

let has_report_to =
  fun
  | Option.None
  | Some([]) => false
  | _ => true;

let to_endpoint = uri => {|{ "url": "|} ++ uri ++ {|" }|};
let csp_endpoint = "csp-endpoint";

let report_to_header = directives => (
  "report-to",
  switch (directives) {
  | {report_to: Option.None | Some([]), _} => ""
  | {report_to: Some(uris), _} =>
    let endpoints = uris |> List.map(to_endpoint) |> String.concat(", ");
    {|{
  "group": "|}
    ++ csp_endpoint
    ++ {|",
  "max_age": 10886400,
  "endpoints": [|}
    ++ endpoints
    ++ {|]
}|};
  },
);

let scheme_to_string =
  fun
  | `HTTP => "http:"
  | `HTTPS => "https:"
  | `Data => "data:"
  | `Mediastream => "mediastream:"
  | `Blob => "blob:"
  | `Filesystem => "filesystem:";

let algo_to_string =
  fun
  | `SHA256 => "sha256"
  | `SHA384 => "sha384"
  | `SHA512 => "sha512";

let src_to_string =
  fun
  | Host(host) => host
  | Scheme(scheme) => scheme_to_string(scheme)
  | Self => "'self'"
  | Unsafe_eval => "'unsafe-eval'"
  | Unsafe_hashes => "'unsafe-hashes'"
  | Unsafe_inline => "'unsafe-inline'"
  | None => "'none'"
  | Nonce(base64) => "'nonce-" ++ base64 ++ "'"
  | [@implicit_arity] Hash(algo, base64) =>
    "'" ++ algo_to_string(algo) ++ "-" ++ base64 ++ "'";

let src_list_to_string = (~name) =>
  fun
  | Option.None
  | Some([]) => ""
  | Some(src_list) => {
      let src_list =
        src_list |> List.map(src_to_string) |> String.concat(" ");
      name ++ " " ++ src_list;
    };

let string_list_to_string = (~name) =>
  fun
  | Option.None
  | Some([]) => ""
  | Some(list) => name ++ " " ++ String.concat(" ", list);

let to_header =
    (
      ~report_only=false,
      {
        child_src,
        connect_src,
        default_src,
        font_src,
        frame_src,
        img_src,
        manifest_src,
        media_src,
        object_src,
        prefetch_src,
        script_src,
        script_src_elem,
        script_src_attr,
        style_src,
        style_src_elem,
        style_src_attr,
        worker_src,
        base_uri,
        plugin_types,
        form_action,
        navigate_to,
        report_to,
        block_all_mixed_content,
      },
    ) => {
  let name =
    if (report_only) {"content-security-policy-report-only"} else {
      "content-security-policy"
    };

  let directives = [
    src_list_to_string(~name="child-src", child_src),
    src_list_to_string(~name="connect-src", connect_src),
    src_list_to_string(~name="default-src", Some(default_src)),
    src_list_to_string(~name="font-src", font_src),
    src_list_to_string(~name="frame-src", frame_src),
    src_list_to_string(~name="img-src", img_src),
    src_list_to_string(~name="manifest-src", manifest_src),
    src_list_to_string(~name="media-src", media_src),
    src_list_to_string(~name="object-src", object_src),
    src_list_to_string(~name="prefetch-src", prefetch_src),
    src_list_to_string(~name="script-src", script_src),
    src_list_to_string(~name="script_src-elem", script_src_elem),
    src_list_to_string(~name="script_src-attr", script_src_attr),
    src_list_to_string(~name="style-src", style_src),
    src_list_to_string(~name="style_src-elem", style_src_elem),
    src_list_to_string(~name="style_src-attr", style_src_attr),
    src_list_to_string(~name="worker-src", worker_src),
    src_list_to_string(~name="base-uri", base_uri),
    string_list_to_string(~name="plugin-types", plugin_types),
    src_list_to_string(~name="form-action", form_action),
    src_list_to_string(~name="navigate-to", navigate_to),
    string_list_to_string(~name="report-uri", report_to),
    switch (block_all_mixed_content) {
    | Some(true) => "block-all-mixed-content"
    | _ => ""
    },
  ];

  let directives =
    if (has_report_to(report_to)) {
      ["report-to " ++ csp_endpoint, ...directives];
    } else {
      directives;
    };

  (name, directives |> List.filter((!=)("")) |> String.concat("; "));
};
