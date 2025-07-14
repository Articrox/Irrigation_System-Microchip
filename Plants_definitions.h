// Lookup table for our plants
const PlantMoistureThresholds PLANT_THRESHOLDS[] = {
    // name, moisture_low, moisture_ideal_low, moisture_ideal_high, moisture_high
    {"peppermint", 40, 50, 70, 80},  // Peppermint likes moist but not soggy soil
    {"tulip",      30, 40, 60, 70},  // Tulips prefer slightly drier conditions
    {"basil",      40, 55, 75, 85},  // Basil enjoys consistently moist soil
    // Add more plants as needed
};