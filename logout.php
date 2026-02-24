<?php
session_start();

/* Destroy session */
$_SESSION = [];
session_destroy();

/* Return JSON */
header('Content-Type: application/json');
echo json_encode(["success" => true]);
?>