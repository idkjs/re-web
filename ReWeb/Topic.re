type key =
  | Key(int);

module Cache =
  Cache.Ephemeral({
    type t = key;
    let equal = (Key(k1), Key(k2)) => k1 == k2;
    let hash = Hashtbl.seeded_hash;
  });

type t('a) = Cache.t((Lwt_stream.t('a), option('a) => unit));

/* A pair of (subscription key, topic) */
type subscription('a) = (key, t('a));

let make = Cache.make;

let num_subscribers = Cache.length;

let publish = (topic, ~msg) =>
  Cache.iter(topic, ~f=(_, (_, push)) => push(Some(msg)));

let publish_from = ((Key(key), topic), ~msg) =>
  Cache.iter(topic, ~f=(Key(cache_key), (_, push)) =>
    if (key != cache_key) {
      push(Some(msg));
    }
  );

let pull = ((key, topic), ~timeout) => {
  open Lwt.Syntax;
  let* (stream, _) = Cache.find(topic, ~key);
  Lwt.pick([
    Lwt_stream.get(stream),
    timeout |> Lwt_unix.sleep |> Lwt.map @@ (() => None),
  ]);
};

let () = Random.self_init();

let subscribe = topic => {
  let stream_push = Lwt_stream.create();
  let key = Key(Random.bits());
  open Lwt.Syntax;
  let+ () = Cache.add(topic, ~key, stream_push);
  (key, topic);
};
