$fn = $preview?6:120;

t = 3;

wpcb = 200;
hpcb = 60;
bpcb = 4;

wo = t + bpcb/2 + wpcb + bpcb/2 + t;
ho = t + bpcb/2 + hpcb + bpcb/2 + t;

w_text = 3.2;
s_thick = 8;
pcb_dist = 4;

depth = 50;

difference() {
    translate([0, 0, depth / 2]) cube([wo, ho, depth], center = true);
    
    translate([0, 0, depth + w_text])
        rotate([0, 180, 0])
            linear_extrude(depth)
                text( "Ms ICP",size=44,font="Roboto:style=Black" , halign = "center", valign="center");
    
    translate([0, 0, depth / 2 + s_thick + w_text]) cube([wpcb - bpcb, hpcb - bpcb, depth], center = true);
    
    translate([0, 0, depth / 2 + s_thick + w_text + pcb_dist]) cube([bpcb + wpcb, bpcb + hpcb, depth], center = true);

    translate([-wo/2, -(ho/2)/2,depth/3*2]) rotate([0,90,0]) cylinder(h = t, d = 11); 
}

/*
translate([0.0, 0, 20]) linear_extrude(height = 10, center = true, scale=1)
    rotate([0, 180,0]) import(file = "../kicad/msicp-brd.svg", center = true);
*/