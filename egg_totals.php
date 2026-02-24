<?php
include 'db.php';
header('Content-Type: application/json');

// Get the date from GET params if provided
$date = $_GET['date'] ?? '';

// Table name
$table = "egg_counts";

if ($date) {
    // Filter totals by specific date
    $stmt = $conn->prepare("
        SELECT 
            COALESCE(SUM(small),0) AS small,
            COALESCE(SUM(medium),0) AS medium,
            COALESCE(SUM(large),0) AS large
        FROM $table
        WHERE date = ?
    ");
    $stmt->bind_param("s", $date);
} else {
    // Total of all dates
    $stmt = $conn->prepare("
        SELECT 
            COALESCE(SUM(small),0) AS small,
            COALESCE(SUM(medium),0) AS medium,
            COALESCE(SUM(large),0) AS large
        FROM $table
    ");
}

$stmt->execute();
$result = $stmt->get_result()->fetch_assoc();

// Return totals as JSON
echo json_encode([
    'small'  => (int)$result['small'],
    'medium' => (int)$result['medium'],
    'large'  => (int)$result['large'],
    'total'  => (int)$result['small'] + (int)$result['medium'] + (int)$result['large']
]);
?>
