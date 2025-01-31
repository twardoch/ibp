# itkn4iihc

Image filter plugin for itkn4iihc

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_itkn4iihc](../assets/img_out/dog_itkn4iihc.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_itkn4iihc](../assets/img_out/female_itkn4iihc.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_itkn4iihc](../assets/img_out/male_itkn4iihc.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_itkn4iihc](../assets/img_out/tree_itkn4iihc.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.itkn4iihc
bypass=false
gridsize=7
outputmode=75

[info]
description=Image filter plugin for itkn4iihc
fileType=ibp.imagefilterlist
nFilters=1
name=ITK N4 IIH Correction


```
