<?php
include 'db.php';
header('Content-Type: application/json');

$date = $_GET['date'] ?? '';

if ($date) {
    $sql = "SELECT date, small, medium, large FROM egg_counts WHERE date='$date'";
} else {
    $sql = "SELECT date, small, medium, large FROM egg_counts ORDER BY date DESC";
}

$res = $conn->query($sql);
$data = [];

while ($row = $res->fetch_assoc()) {
    $data[] = $row;
}

echo json_encode($data);
