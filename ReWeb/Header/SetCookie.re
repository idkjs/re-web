type same_site =
  | None
  | Strict
  | Lax;
type t = (string, string);

let make =
    (
      ~max_age=?,
      ~secure=ReWeb__Config.Default.secure,
      ~http_only=true,
      ~domain=?,
      ~path=?,
      ~same_site=Lax,
      ~name,
      value,
    ) => {
  let same_site =
    switch (same_site) {
    | None => "None"
    | Strict => "Strict"
    | Lax => "Lax"
    };

  Directive.(
    name,
    value
    ++ int(~name="Max-Age", max_age)
    ++ bool(~name="Secure", secure)
    ++ bool(~name="HttpOnly", http_only)
    ++ string(~name="Domain", domain)
    ++ string(~name="Path", path)
    ++ "; SameSite="
    ++ same_site,
  );
};

let name = fst;
let to_header = ((name, value)) => ("set-cookie", name ++ "=" ++ value);

let of_header = value =>
  switch (String.split_on_char('=', value)) {
  | []
  | [_] => Option.None
  | [name, ...value] => Some((name, String.concat("=", value)))
  };

let value = snd;
