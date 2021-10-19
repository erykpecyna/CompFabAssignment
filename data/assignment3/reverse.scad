layers = 3;
hexagonsize = 2;
layerotation = 35;

module hexagon(x, y) {
    $fn = 6;
    translate([x, y, 0])
    rotate([0,0,30])
    difference() {
        cylinder(h = 2, r = 12);
        scale([0.8,0.8,1.1])
        cylinder(h=2, r=12);
    }
}

module meshline(size) {
    t = size/2;
    for (dx=[-t:19:t]) {
        hexagon(dx,0);
    }
}

module meshrows(size) {
    t = size/2;
    for (dy=[-t:32:t]) {
        translate([0,dy,0])
            meshline(size);
    }
}

module honeycomb(size) {
    meshrows(size);
    translate([10,16,0])
    meshrows(size);
}

module bordercircle() {
    $fn = 40;
    difference() {
        cylinder(h=2, r=81);
        scale([0.95,0.95,1.1])
        cylinder(h=2, r=80);
    }
}

module layer(rot) {
    union() {
        intersection() {
            cylinder(h=2,r=80);
            rotate([0,0,rot])
            scale([hexagonsize, hexagonsize, 1])
            honeycomb(160 / hexagonsize);
        }
        bordercircle();
    }
}

for (l=[0:1:layers]) {
    translate([0,0,2*l])
    layer(l * layerotation);
}


