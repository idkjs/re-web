type privately = {
  must_revalidate: option(bool),
  max_age: option(int),
};

type publicly = {
  no_transform: option(bool),
  proxy_revalidate: option(bool),
  s_maxage: option(int),
};

type t =
  | No_store
  | No_cache
  | Private(privately)
  | Public(privately, publicly);

let private = (~must_revalidate=?, ~max_age=?, ()) =>
  Private({must_revalidate, max_age});

let public =
    (
      ~must_revalidate=?,
      ~max_age=?,
      ~no_transform=?,
      ~proxy_revalidate=?,
      ~s_maxage=?,
      (),
    ) =>
  [@implicit_arity]
  Public(
    {must_revalidate, max_age},
    {no_transform, proxy_revalidate, s_maxage},
  );

let commalist = list =>
  switch (List.filter((!=)(""), list)) {
  | [] => ""
  | directives => "," ++ String.concat(",", directives)
  };

let privately_to_string = ({must_revalidate, max_age}) =>
  commalist([
    if (Option.value(~default=false, must_revalidate)) {
      "must-revalidate";
    } else {
      "";
    },
    max_age
    |> Option.map(int => int |> string_of_int |> (++)("max-age="))
    |> Option.value(~default=""),
  ]);

let publicly_to_string = ({no_transform, proxy_revalidate, s_maxage}) =>
  commalist([
    if (Option.value(~default=false, no_transform)) {
      "no-transform";
    } else {
      "";
    },
    if (Option.value(~default=false, proxy_revalidate)) {
      "proxy-revalidate";
    } else {
      "";
    },
    s_maxage
    |> Option.map(int => int |> string_of_int |> (++)("s-maxage="))
    |> Option.value(~default=""),
  ]);

let to_string =
  fun
  | No_store => "no-store"
  | No_cache => "no-cache"
  | Private(privately) => "private" ++ privately_to_string(privately)
  | [@implicit_arity] Public(privately, publicly) =>
    "public"
    ++ privately_to_string(privately)
    ++ publicly_to_string(publicly);
