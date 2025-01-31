# inpaintingiihc

Image filter plugin for inpaintingiihc

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_inpaintingiihc](../assets/img_out/dog_inpaintingiihc.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_inpaintingiihc](../assets/img_out/female_inpaintingiihc.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_inpaintingiihc](../assets/img_out/male_inpaintingiihc.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_inpaintingiihc](../assets/img_out/tree_inpaintingiihc.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.inpaintingiihc
bypass=false
maskexpansion=75
noisereduction=7500.0
outputmode=75

[info]
description=Image filter plugin for inpaintingiihc
fileType=ibp.imagefilterlist
nFilters=1
name=Inpainting IIH Correction


```
