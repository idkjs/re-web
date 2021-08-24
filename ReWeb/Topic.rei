/** A topic for messages of type ['a]. */
type t('a);

/** A subscription to a topic of type ['a]. */

type subscription('a);

/** [make()] is a new topic. Typically you will create these at a scope
    that can pass them to any parts of the application that need them. */

let make: unit => t(_);

/** [num_subscribers(topic)] is a count of the subscribers of the given
    [topic]. */

let num_subscribers: t('a) => Lwt.t(int);

/** [publish(topic, ~msg)] publishes [msg] onto the [topic]. This
    broadcasts the [msg] to all subscribers of the [topic]. */

let publish: (t('a), ~msg: 'a) => Lwt.t(unit);

/** [publish_from(subscription, ~msg)] publishes [msg] to the topic that
    [subscription] subscribes to, ensuring the message is sent to all
    subscribers {i except} the sender [subscription]. */

let publish_from: (subscription('a), ~msg: 'a) => Lwt.t(unit);

/** [pull(subscription, ~timeout)] is a message from the topic
    subscribed to by [subscription] if there is one within the
    [timeout] (in seconds); else [None]. */

let pull: (subscription('a), ~timeout: float) => Lwt.t(option('a));

/** [subscribe(topic)] is a subscription to the [topic]. Note that
    subscriptions automatically get unsubscribed as soon as the
    [subscription] key goes out of scope. */

let subscribe: t('a) => Lwt.t(subscription('a));
