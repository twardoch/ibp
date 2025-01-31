# contrastpreservinggrayscale

Convert the image to grayscale preserving the contrast

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_contrastpreservinggrayscale](../assets/img_out/dog_contrastpreservinggrayscale.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_contrastpreservinggrayscale](../assets/img_out/female_contrastpreservinggrayscale.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_contrastpreservinggrayscale](../assets/img_out/male_contrastpreservinggrayscale.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_contrastpreservinggrayscale](../assets/img_out/tree_contrastpreservinggrayscale.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.contrastpreservinggrayscale
bypass=false

[info]
description=Convert the image to grayscale preserving the contrast
fileType=ibp.imagefilterlist
nFilters=1
name=Contrast Preserving Grayscale


```
