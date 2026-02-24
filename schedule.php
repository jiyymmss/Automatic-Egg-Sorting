<?php
session_start();
header('Content-Type: application/json');
include 'db.php';

/* GET schedules */
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    $res = $conn->query("SELECT id, feed_time FROM feeding_schedule ORDER BY feed_time");
    echo json_encode($res->fetch_all(MYSQLI_ASSOC));
    exit;
}

/* POST add schedule */
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents("php://input"), true);
    $time = $conn->real_escape_string($data['feed_time']);
    $conn->query("INSERT INTO feeding_schedule (feed_time) VALUES ('$time')");
    echo json_encode(["success" => true]);
    exit;
}

/* DELETE schedule */
if ($_SERVER['REQUEST_METHOD'] === 'DELETE') {
    $data = json_decode(file_get_contents("php://input"), true);
    $id = (int)$data['id'];

    $conn->query("DELETE FROM feeding_schedule WHERE id=$id");
    echo json_encode(["success" => true]);
    exit;
}
