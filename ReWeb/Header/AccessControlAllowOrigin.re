type t =
  | All
  | One(string);

let to_header = t => (
  "access-control-allow-origin",
  switch (t) {
  | All => "*"
  | One(origin) => origin
  },
);
