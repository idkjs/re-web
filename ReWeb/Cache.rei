/** The cache module here uses Lwt to ensure that caches are accessed
    serially to prevent inconsistent data accesses. */;

/** Aside from [make] and [access], the rest of the functions in this
    interface are similar to the ones in [Hashtbl], so you can use those
    as a reference. */

module type S = {
  type key;
  module Table: Hashtbl.SeededS with type key = key;
  type t('a);

  /** [access(cache, f)] runs the function [f] on the hash table
      contents of the [cache], returning the result value. It can be
      used to both read from and write to the cache.

      [f(table)] is a callback which has locked access to the hash
      [table] contained in the [cache] and can do anything with it,
      including multiple operations like checking if the cache contains
      a key, then adding it or not.

      Since this does lock the cache, it is preferable to not do
      long-running operations here, and to finish and unlock as quickly
      as possible. */

  let access: (t('a), Table.t('a) => 'b) => Lwt.t('b);

  let add: (t('a), ~key: key, 'a) => Lwt.t(unit);
  let find: (t('a), ~key: key) => Lwt.t('a);
  let find_opt: (t('a), ~key: key) => Lwt.t(option('a));
  let iter: (t('a), ~f: (key, 'a) => unit) => Lwt.t(unit);
  let length: t('a) => Lwt.t(int);

  /** [make()] allocates a new cache value. */

  let make: unit => t('a);

  let mem: (t('a), ~key: key) => Lwt.t(bool);
  let remove: (t('a), ~key: key) => Lwt.t(unit);
  let reset: t('a) => Lwt.t(unit);
};

/** [Ephemeral(Key)] is a module that manages an ephemeral concurrent
    cache. An ephemeral cache is one whose bindings are deleted as soon
    as its keys go out of scope.

    {i Note} that this module overrides a couple of functions to clean
    the cache i.e. remove dead bindings (modulo a full GC cycle):

    - [length] always cleans the cache
    - [find] probabilistically cleans the cache with a 1/8 likelihood */

module Ephemeral: (Key: Hashtbl.SeededHashedType) => S with type key = Key.t;

/** [InMemory(Key)] is a module that manages a concurrent in-memory
    cache. */

module InMemory: (Key: Hashtbl.SeededHashedType) => S with type key = Key.t;

/** [IntKey] is a module that can be used to create a cache with [int]
    keys. */

module IntKey: Hashtbl.SeededHashedType with type t = int;

/** [Make(T)] is a module that manages a concurrent cache with the
    persistence characteristics offered by the [Table] module. */

module Make: (T: Hashtbl.SeededS) => S with type key = T.key;

/** [StringKey] is a module that can be used to create a cache with
    [string] keys. */

module StringKey: Hashtbl.SeededHashedType with type t = string;
