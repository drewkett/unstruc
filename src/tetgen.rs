extern crate libc;
extern crate cgmath;

use cgmath::Point3;
use ::Grid;

extern {
    fn new_tetgenio() -> *const libc::c_void;
    fn set_mesh_dim(tg: *const libc::c_void,mesh_dim : i32);
    fn set_firstnumber(tg: *const libc::c_void,firstnumber : i32);
    fn set_numberofpoints(tg: *const libc::c_void,numberofpoints : i32);
    fn set_pointlist(tg: *const libc::c_void, pointlist : *const Point3<f64>);
}

pub struct TetGenIO {
    tetgenio : *const libc::c_void,
}

impl TetGenIO {
    pub fn new(g : Grid) -> TetGenIO {
        let tg = unsafe {
            let tg = new_tetgenio();
            set_mesh_dim(tg,3);
            set_firstnumber(tg,0);

            set_pointlist(tg,g.points.as_ptr());
            tg
        };
        return TetGenIO{tetgenio: tg}
    }

    pub fn set_mesh_dim(&self, mesh_dim : i32) { unsafe {set_mesh_dim(self.tetgenio,mesh_dim); }}
    pub fn set_firstnumber(&self, firstnumber : i32) { unsafe {set_firstnumber(self.tetgenio,firstnumber);} }
    pub fn set_points(&self, points : &Vec<Point3<f64>>) {
        unsafe {
            set_numberofpoints(self.tetgenio,points.len() as i32);
            set_pointlist(self.tetgenio,points.as_ptr());
        }
    }
}

// Grid volgrid_from_surface(Grid const& surface, const std::vector<Point>& holes, double min_ratio) {
//    std::vector <Point> holes;
//    double min_ratio = 0;
//    return volgrid_from_surface(surface,holes,min_ratio);
// }
