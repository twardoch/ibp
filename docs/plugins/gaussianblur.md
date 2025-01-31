# gaussianblur

Blurs the image using a smooth gaussian distribution

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_gaussianblur](../assets/img_out/dog_gaussianblur.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_gaussianblur](../assets/img_out/female_gaussianblur.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_gaussianblur](../assets/img_out/male_gaussianblur.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_gaussianblur](../assets/img_out/tree_gaussianblur.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.gaussianblur
bypass=false
bluralpha=1
blurrgb=1
radius=35

[info]
description=Blurs the image using a smooth gaussian distribution
fileType=ibp.imagefilterlist
nFilters=1
name=Gaussian Blur


```
