open Alcotest

module Let = ReWeb__Let
module Topic = ReWeb.Topic

let s = "ReWeb.Topic", [
  Alcotest_lwt.test_case "make, subscribe, publish, pull" `Quick begin fun _ () ->
    let msg = 0 in
    let topic = Topic.make () in
    let open Let.Lwt in

    (* First 'we' subscribe to the topic *)
    let* subscription = Topic.subscribe topic in

    (* Then 'someone' publishes something to the topic *)
    let* () = Topic.publish topic ~msg in

    (* While the topic is streaming out data, check we got what we
       expected *)
    let+ msg_option = Topic.pull subscription ~timeout:1. in
    check (option int) "" (Some msg) msg_option
  end;
]

