r = 20;

module tip(size) {
    $fn = 40;
    if (size > 3) {
        union() {
            sphere(size);
            translate([0,0,size*(3/5)])
            tip(size * 0.8);
        }
    }
}

union() {
    translate([0,0,2*r])
    tip(r);
    for (i=[0:20:359]) {
        angle = i;
        dx = r * cos(angle);
        dy = r * sin(angle);
        $fn = 20;
        translate([dx,dy,2*r])
            sphere(1);
    }
    $fn = 20;
    cylinder(h = r, r = 2);
    scale([r,r,1])
    sphere(0.8);
}