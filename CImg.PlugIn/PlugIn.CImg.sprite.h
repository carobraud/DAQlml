#ifndef PLUGIN_CIMG_SPRITE
#define PLUGIN_CIMG_SPRITE
//complies with CImg version 1.3.4

//! Add a sprite image in the instance image.
    /**
       \param sprite Sprite image.
       \param x0 X-coordinate of the sprite position.
       \param y0 Y-coordinate of the sprite position.
       \param z0 Z-coordinate of the sprite position.
       \param v0 V-coordinate of the sprite position.
       \note
       - Clipping is supported as it is adapted from \c CImg::draw_image.
    **/
    template<typename image_typeT>
      CImg<image_typeT>& add_sprite(const CImg<image_typeT>& sprite,
                       const int x0, const int y0=0, const int z0=0, const int v0=0) {
      if (!is_empty()) {
        if (!sprite)
          throw CImgArgumentException("CImg<%s>::draw_image() : Specified sprite image (%u,%u,%u,%u,%p) is empty.",
                                      pixel_type(),sprite.width,sprite.height,sprite.depth,sprite.dim,sprite.data);
        const bool bx=(x0<0), by=(y0<0), bz=(z0<0), bv=(v0<0);
        const int
          lX = sprite.width() - (x0+sprite.width()>width()?x0+sprite.width()-width():0) + (bx?x0:0),
          lY = sprite.height() - (y0+sprite.height()>height()?y0+sprite.height()-height():0) + (by?y0:0),
          lZ = sprite.depth() - (z0+sprite.depth()>depth()?z0+sprite.depth()-depth():0) + (bz?z0:0),
          lV = sprite.spectrum() - (v0+sprite.spectrum()>spectrum()?v0+sprite.spectrum()-spectrum():0) + (bv?v0:0);
        const image_typeT
          *ptrs = sprite.data -
          (bx?x0:0) -
          (by?y0*sprite.width():0) -
          (bz?z0*sprite.width()*sprite.height():0) -
          (bv?v0*sprite.width()*sprite.height()*sprite.depth():0);
        const unsigned int
          offX = width-lX,                soffX = sprite.width-lX,
          offY = width*(height-lY),       soffY = sprite.width*(sprite.height-lY),
          offZ = width*height*(depth-lZ), soffZ = sprite.width*sprite.height*(sprite.depth-lZ);
        
        T *ptrd = data(x0<0?0:x0,y0<0?0:y0,z0<0?0:z0,v0<0?0:v0);
        if (lX>0 && lY>0 && lZ>0 && lV>0)
          for (int v=0; v<lV; ++v) {
            for (int z=0; z<lZ; ++z) {
              for (int y=0; y<lY; ++y) {
                for (int x=0; x<lX; ++x) *(ptrd++) += (T)*(ptrs++);
                ptrd+=offX; ptrs+=soffX;
              }
              ptrd+=offY; ptrs+=soffY;
            }
            ptrd+=offZ; ptrs+=soffZ;
          }
      }
      return *this;
    }


#endif/*PLUGIN_CIMG_SPRITE*/
