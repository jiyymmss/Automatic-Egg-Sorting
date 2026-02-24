<?php
session_start();
header('Content-Type: application/json');
include 'db.php';

$data = json_decode(file_get_contents("php://input"), true);

if (!$data) {
    echo json_encode(["success" => false, "message" => "Invalid request"]);
    exit;
}

$username = $conn->real_escape_string($data['username']);
$password = $data['password'];

$res = $conn->query("SELECT * FROM users WHERE username='$username' LIMIT 1");

if ($res && $res->num_rows === 1) {
    $user = $res->fetch_assoc();

    if ($password === $user['password']) {
        $_SESSION['user'] = $user['id'];
        echo json_encode(["success" => true]);
        exit;
    }
}

echo json_encode(["success" => false, "message" => "Invalid username or password"]);
