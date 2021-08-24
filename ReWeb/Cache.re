module type S = {
  type key;
  module Table: Hashtbl.SeededS with type key = key;
  type t('a);

  let access: (t('a), Table.t('a) => 'b) => Lwt.t('b);
  let add: (t('a), ~key: key, 'a) => Lwt.t(unit);
  let find: (t('a), ~key: key) => Lwt.t('a);
  let find_opt: (t('a), ~key: key) => Lwt.t(option('a));
  let iter: (t('a), ~f: (key, 'a) => unit) => Lwt.t(unit);
  let length: t('a) => Lwt.t(int);
  let make: unit => t('a);
  let mem: (t('a), ~key: key) => Lwt.t(bool);
  let remove: (t('a), ~key: key) => Lwt.t(unit);
  let reset: t('a) => Lwt.t(unit);
};

module Make = (T: Hashtbl.SeededS) => {
  module Table = T;
  type key = Table.key;
  type t('a) = (Table.t('a), Lwt_mutex.t);

  let make = () => (Table.create(~random=true, 32), Lwt_mutex.create());

  let access = ((table, lock), f) =>
    Lwt_mutex.with_lock(lock) @@ (() => table |> f |> Lwt.return);

  let add = (t, ~key, value) =>
    access(t) @@ (table => Table.add(table, key, value));

  let find = (t, ~key) => access(t) @@ (table => Table.find(table, key));

  let find_opt = (t, ~key) =>
    access(t) @@ (table => Table.find_opt(table, key));

  let iter = (t, ~f) => access(t) @@ (table => Table.iter(f, table));
  let length = t => access(t, Table.length);
  let mem = (t, ~key) => access(t) @@ (table => Table.mem(table, key));
  let remove = (t, ~key) =>
    access(t) @@ (table => Table.remove(table, key));
  let reset = t => access(t, Table.reset);
};

module Ephemeral = (Key: Hashtbl.SeededHashedType) => {
  module EphemeralHashtbl = Ephemeron.K1.MakeSeeded(Key);
  include Make(EphemeralHashtbl);

  let () = Random.self_init();

  let find = (t, ~key) =>
    access(
      t,
      table => {
        /* Instead of keeping a counter and cleaning exactly every 8 times,
           we probabilistically clean 1/8 times. */
        if (Random.int(8) == 0) {
          EphemeralHashtbl.clean(table);
        };
        EphemeralHashtbl.find(table, key);
      },
    );

  let length = t =>
    access(
      t,
      table => {
        EphemeralHashtbl.clean(table);
        EphemeralHashtbl.length(table);
      },
    );
};

module InMemory = (Key: Hashtbl.SeededHashedType) =>
  Make((Hashtbl.MakeSeeded(Key)));

module SimpleKey = {
  let equal = (==);
  let hash = Hashtbl.seeded_hash;
};

module IntKey = {
  include SimpleKey;
  type t = int;
};

module StringKey = {
  include SimpleKey;
  type t = string;
};
