layers = 1;
hexagonsize = 1;
layerotation = 0;

module hexagon() {
    $fn = 6;
    difference() {
        cylinder(h = 2, r = 12);
        scale([0.8,0.8,1])
        cylinder(h=2, r=12);
    }
}

hexagon();

$fn = 20;
translate([20,20,0])
    cylinder(h=2, r=12,center=true);