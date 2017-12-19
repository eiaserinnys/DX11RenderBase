#include "pch.h"

#include <png.h>
#include "RenderConfig.h"
#include "DX11SaveTextureToPng.h"

using namespace std;

static DXGI_FORMAT EnsureNotTypeless( DXGI_FORMAT fmt )
{
    // Assumes UNORM or FLOAT; doesn't use UINT or SINT
    switch( fmt )
    {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case DXGI_FORMAT_R32G32B32_TYPELESS:    return DXGI_FORMAT_R32G32B32_FLOAT;
    case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_UNORM;
    case DXGI_FORMAT_R32G32_TYPELESS:       return DXGI_FORMAT_R32G32_FLOAT;
    case DXGI_FORMAT_R10G10B10A2_TYPELESS:  return DXGI_FORMAT_R10G10B10A2_UNORM;
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:     return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_R16G16_TYPELESS:       return DXGI_FORMAT_R16G16_UNORM;
    case DXGI_FORMAT_R32_TYPELESS:          return DXGI_FORMAT_R32_FLOAT;
    case DXGI_FORMAT_R8G8_TYPELESS:         return DXGI_FORMAT_R8G8_UNORM;
    case DXGI_FORMAT_R16_TYPELESS:          return DXGI_FORMAT_R16_UNORM;
    case DXGI_FORMAT_R8_TYPELESS:           return DXGI_FORMAT_R8_UNORM;
    case DXGI_FORMAT_BC1_TYPELESS:          return DXGI_FORMAT_BC1_UNORM;
    case DXGI_FORMAT_BC2_TYPELESS:          return DXGI_FORMAT_BC2_UNORM;
    case DXGI_FORMAT_BC3_TYPELESS:          return DXGI_FORMAT_BC3_UNORM;
    case DXGI_FORMAT_BC4_TYPELESS:          return DXGI_FORMAT_BC4_UNORM;
    case DXGI_FORMAT_BC5_TYPELESS:          return DXGI_FORMAT_BC5_UNORM;
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:     return DXGI_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:     return DXGI_FORMAT_B8G8R8X8_UNORM;
    case DXGI_FORMAT_BC7_TYPELESS:          return DXGI_FORMAT_BC7_UNORM;
    default:                                return fmt;
    }
}

HRESULT DX11SaveTextureToPng::Save(
	ID3D11Device* dev,
	ID3D11DeviceContext* devCtx,
	ID3D11Texture2D* texture,
	const char* name,
	int& width, int& height, 
	int& cx, int& cy, int& cw, int& ch, 
	bool crop)
{
	HRESULT hr;    
	D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc( &desc );

	ID3D11Texture2D* staging = NULL;

    if (desc.SampleDesc.Count > 1)
    {
        // MSAA content must be resolved before being copied to a staging texture
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        ID3D11Texture2D* pTemp;
        hr = dev->CreateTexture2D( &desc, 0, &pTemp);
        if (FAILED(hr)) { return hr; }

        DXGI_FORMAT fmt = EnsureNotTypeless( desc.Format );

        UINT support = 0;
        hr = dev->CheckFormatSupport( fmt, &support );
        if ( FAILED(hr) ) { return hr; }

        if (!(support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE) ) { return E_FAIL; }
        for( UINT item = 0; item < desc.ArraySize; ++item )
        {
            for( UINT level = 0; level < desc.MipLevels; ++level )
            {
                UINT index = D3D11CalcSubresource( level, item, desc.MipLevels );
                devCtx->ResolveSubresource(pTemp, index, texture, index, fmt);
            }
        }

        desc.BindFlags = 0;
        desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.Usage = D3D11_USAGE_STAGING;

        hr = dev->CreateTexture2D(&desc, 0, &staging);
        if ( FAILED(hr) ) { return hr; }

        devCtx->CopyResource(staging, pTemp);
    }
    else if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
    {
        // Handle case where the source is already a staging texture we can use directly
        //pStaging = pTexture;
    }
    else
    {
        // Otherwise, create a staging texture from the non-MSAA source
        desc.BindFlags = 0;
        desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.Usage = D3D11_USAGE_STAGING;

        hr = dev->CreateTexture2D( &desc, 0, &staging);
        if ( FAILED(hr) ) { return hr; }

        devCtx->CopyResource(staging, texture);
    }

	if (staging != NULL)
	{
		hr = SaveToPng(devCtx, staging, name, width, height, cx, cy, cw, ch, crop);

		staging->Release();

		return hr;
	}

    return S_OK;
}

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
	uint8_t alpha;
} pixel_t;

/* A picture. */
    
typedef struct  {
    pixel_t *pixels;
    size_t width;
    size_t height;
	size_t pitch;
} bitmap_t;

static pixel_t * pixel_at (bitmap_t * bitmap, int x, int y)
{
    return bitmap->pixels + bitmap->pitch * y + x;
}

static int save_png_to_file (bitmap_t *bitmap, const char *path)
{
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;
    /* "status" contains the return value of this function. At first
       it is set to a value which means 'failure'. When the routine
       has finished its work, it is set to a value which means
       'success'. */
    int status = -1;
    /* The following number is set by trial and error only. I cannot
       see where it it is documented in the libpng manual.
    */
    int pixel_size = 4;
    int depth = 8;
    
    fp = fopen (path, "wb");
    if (! fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }
    
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }
    
    /* Set up error handling. */

    if (setjmp (png_jmpbuf (png_ptr))) {
        goto png_failure;
    }
    
    /* Set image attributes. */

    png_set_IHDR (png_ptr,
                  info_ptr,
                  bitmap->width,
                  bitmap->height,
                  depth,
                  PNG_COLOR_TYPE_RGBA,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    
    /* Initialize rows of PNG. */

    row_pointers = (png_byte**) png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
    for (y = 0; y < bitmap->height; ++y) {
        png_byte *row = (png_byte *)
            png_malloc (png_ptr, sizeof (uint8_t) * bitmap->width * pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < bitmap->width; ++x) {
            pixel_t * pixel = pixel_at (bitmap, x, y);
            *row++ = pixel->red;
            *row++ = pixel->green;
            *row++ = pixel->blue;
			*row++ = pixel->alpha;
        }
    }
    
    /* Write the image data to "fp". */

    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* The routine has successfully written the file, so we set
       "status" to a value which indicates success. */

    status = 0;
    
    for (y = 0; y < bitmap->height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
    
 png_failure:
 png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
 png_create_write_struct_failed:
    fclose (fp);
 fopen_failed:
    return status;
}

HRESULT DX11SaveTextureToPng::SaveToPng(
	ID3D11DeviceContext* devCtx, 
	ID3D11Texture2D* staging,
	const char* name,
	int& halfWidth, int& halfHeight, 
	int& cx, int& cy, 
	int& cw, int& ch, 
	bool crop)
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = devCtx->Map(staging, 0, D3D11_MAP_READ, 0, &mapped);
	if (FAILED(hr)) { return hr; }

	auto sptr = reinterpret_cast<const uint32_t*>(mapped.pData);
	if (!sptr)
	{
		devCtx->Unmap(staging, 0);
		staging->Release();
		return E_POINTER;
	}

	D3D11_TEXTURE2D_DESC desc;
    staging->GetDesc(&desc);

	bitmap_t bitmap;
	bitmap.pixels = (pixel_t*) sptr;
	bitmap.width = desc.Width;
	bitmap.height = desc.Height;
	bitmap.pitch = mapped.RowPitch / 4;

#	if FILTER_READY
	for (int i = 0; i < desc.Height; ++i)
	{
		for (int j = 0; j < desc.Width; ++j)
		{
			if (pixel_at(&bitmap, j, i)->alpha == 0)
			{
				for (int k = -1; k <= 1; ++k)
				{
					for (int l = -1; l <= 1; ++l)
					{
						if (j + l >= 0 && j + l < desc.Width &&
							i + k >= 0 && i + k < desc.Height)
						{
							if (pixel_at(&bitmap, j + l, i + k)->alpha > 0)
							{
								*pixel_at(&bitmap, j, i) = *pixel_at(&bitmap, j + l, i + k);
							}
						}
					}
				}
			}
		}
	}
#	endif

	// 자 이걸 반으로 줄인다
	halfWidth = desc.Width / 2;
	halfHeight = desc.Height / 2;

	vector<pixel_t> halfSized;
	halfSized.resize(halfWidth * halfHeight);
	for (int i = 0; i < halfHeight; ++i)
	{
		for (int j = 0; j < halfWidth; ++j)
		{
			pixel_t* pixels[] = {
				pixel_at(&bitmap, j * 2 + 0, i * 2 + 0),
				pixel_at(&bitmap, j * 2 + 0, i * 2 + 1),
				pixel_at(&bitmap, j * 2 + 1, i * 2 + 0),
				pixel_at(&bitmap, j * 2 + 1, i * 2 + 1),
			};
			halfSized[i * halfWidth + j].red = 
				min(0xff, ((int)pixels[0]->red + (int)pixels[1]->red + (int)pixels[2]->red + (int)pixels[3]->red) / 4);
			halfSized[i * halfWidth + j].blue = 
				min(0xff, ((int)pixels[0]->blue + (int)pixels[1]->blue + (int)pixels[2]->blue + (int)pixels[3]->blue) / 4);
			halfSized[i * halfWidth + j].green = 
				min(0xff, ((int)pixels[0]->green + (int)pixels[1]->green + (int)pixels[2]->green + (int)pixels[3]->green) / 4);
			halfSized[i * halfWidth + j].alpha = 
				min(0xff, ((int)pixels[0]->alpha + (int)pixels[1]->alpha + (int)pixels[2]->alpha + (int)pixels[3]->alpha) / 4);
		}
	}

	// 크랍할 영역을 찾는다
	int firstNonEmptyY = - 1;
	int bottomEmptyBeginsY = 0;
	int firstNonEmptyX = halfWidth;
	int rightEmptyBeginsX = 0;

	if (crop)
	{
		for (int i = 0; i < halfHeight; ++i)
		{
			bool allEmpty = true;
			for (int j = 0; j < halfWidth; ++j)
			{
				if (firstNonEmptyY == - 1 && halfSized[i * halfWidth + j].alpha != 0)
				{
					firstNonEmptyY = i;
				}
				if (allEmpty && halfSized[i * halfWidth + j].alpha != 0)
				{
					allEmpty = false;
				}
				if (j < firstNonEmptyX && halfSized[i * halfWidth + j].alpha != 0)
				{
					firstNonEmptyX = j;
				}
				if (j > rightEmptyBeginsX && halfSized[i * halfWidth + j].alpha != 0)
				{
					rightEmptyBeginsX = j + 1;
				}
			}

			if (i > bottomEmptyBeginsY && !allEmpty)
			{
				bottomEmptyBeginsY = i + 1;
			}
		}

	}
	else
	{
		firstNonEmptyX = 0;
		firstNonEmptyY = 0;
		rightEmptyBeginsX = halfWidth;
		bottomEmptyBeginsY = halfHeight;
	}

	int croppedWidth = rightEmptyBeginsX - firstNonEmptyX;
	int croppedHeight = bottomEmptyBeginsY - firstNonEmptyY;

	vector<pixel_t> cropped;
	cropped.resize(croppedWidth * croppedHeight);
	for (int i = firstNonEmptyY; i < bottomEmptyBeginsY; ++i)
	{
		for (int j = firstNonEmptyX; j < rightEmptyBeginsX; ++j)
		{
			cropped[(i - firstNonEmptyY) * croppedWidth + j - firstNonEmptyX] = halfSized[i * halfWidth + j];
		}
	}

	bitmap_t resized;
	resized.pixels = &cropped[0];
	resized.width = croppedWidth;
	resized.height = croppedHeight;
	resized.pitch = croppedWidth;

	save_png_to_file(&resized, name);

	devCtx->Unmap(staging, 0);

	cx = firstNonEmptyX;
	cy = firstNonEmptyY;
	cw = croppedWidth;
	ch = croppedHeight;

	return S_OK;
}
