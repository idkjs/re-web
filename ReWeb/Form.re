type decoder('a) = string => result('a, string);

module Field = {
  type t('a) = {
    name: string,
    decoder: decoder('a),
  };

  type list(_, _) =
    | []: list('a, 'a)
    | ::(t('a), list('b, 'c)): list('a => 'b, 'c);

  let make = (name, decoder) => {name, decoder};

  let bool = name =>
    make(name) @@
    (
      string =>
        try(Ok(bool_of_string(string))) {
        | _ => Error("ReWeb.Form.Field.bool: " ++ name)
        }
    );

  let ensure = (pred, {name, decoder}) =>
    make(name) @@
    (
      string => {
        open Let.Result;
        let* a = decoder(string);
        if (pred(a)) {
          Ok(a);
        } else {
          Error("ReWeb.Form.Field.ensure: " ++ name);
        };
      }
    );

  let float = name =>
    make(name) @@
    (
      string =>
        try(Ok(float_of_string(string))) {
        | _ => Error("ReWeb.Form.Field.float: " ++ name)
        }
    );

  let int = name =>
    make(name) @@
    (
      string =>
        try(Ok(int_of_string(string))) {
        | _ => Error("ReWeb.Form.Field.int: " ++ name)
        }
    );

  let string = name => make(name) @@ (string => Ok(string));
};

type t('ctor, 'ty) = {
  fields: Field.list('ctor, 'ty),
  ctor: 'ctor,
};

let rec decode:
  type ctor ty.
    (t(ctor, ty), list((string, list(string)))) => result(ty, string) =
  ({fields, ctor}, fields_assoc) => {
    open! Field;
    switch (fields) {
    | [] => Ok(ctor)
    | [field, ...fields] =>
      switch (List.assoc(field.name, fields_assoc)) {
      | [value] =>
        switch (field.decoder(value)) {
        | Ok(value) =>
          switch (ctor(value)) {
          | ctor => decode({fields, ctor}, fields_assoc)
          | exception _ =>
            Error(
              "ReWeb.Form.decoder: could not decode value for key "
              ++ field.name,
            )
          }
        | Error(string) => Error(string)
        }
      | _ =>
        Error(
          "ReWeb.Form.decoder: could not find single value for key "
          ++ field.name,
        )
      | exception Not_found =>
        Error("ReWeb.Form.decoder: could not find key " ++ field.name)
      }
    };
  };

let decoder = (form, string) =>
  string |> Uri.query_of_encoded |> decode(form);

let empty = {fields: Field.[], ctor: ()};

let encode = (fields, value) =>
  value |> fields |> List.map(((k, v)) => (k, [v])) |> Uri.encoded_of_query;

let make = (fields, ctor) => {fields, ctor};
