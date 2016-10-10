
extern crate libc;

extern {
    fn new_tetgenio() -> *const libc::c_void;
}

pub struct TetGenIO {
    tetgenio : *const libc::c_void,
}

impl TetGenIO {
    pub fn new() -> TetGenIO {
        let tg = unsafe { new_tetgenio() };
        return TetGenIO{tetgenio: tg}
    }
}

