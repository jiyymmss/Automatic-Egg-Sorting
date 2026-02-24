<?php
include 'db.php'; // your database connection

// Get egg type from POST or GET
$eggType = $_POST['type'] ?? $_GET['type'] ?? null;

if ($eggType) {
    $eggType = strtoupper(trim($eggType));

    // Insert a new row for this egg, 1 in the correct column, 0 in others
    switch ($eggType) {
        case 'S':
            $sql = "INSERT INTO egg_counts (date, small, medium, large) VALUES (CURRENT_DATE, 1, 0, 0)";
            break;
        case 'M':
            $sql = "INSERT INTO egg_counts (date, small, medium, large) VALUES (CURRENT_DATE, 0, 1, 0)";
            break;
        case 'L':
            $sql = "INSERT INTO egg_counts (date, small, medium, large) VALUES (CURRENT_DATE, 0, 0, 1)";
            break;
        default:
            echo "Invalid egg type";
            exit;
    }

    if ($conn->query($sql) === TRUE) {
        echo "Egg added successfully";
    } else {
        echo "Error: " . $conn->error;
    }
} else {
    echo "No egg type received";
}

$conn->close();
?>
