# lumakeyer

Apply a matte to the image based on its luminance

| Input | Output |
|--------|--------|
| ![dog](../assets/img_in/dog.jpg) | ![dog_lumakeyer](../assets/img_out/dog_lumakeyer.jpg) |
| ![female](../assets/img_in/female.jpg) | ![female_lumakeyer](../assets/img_out/female_lumakeyer.jpg) |
| ![male](../assets/img_in/male.jpg) | ![male_lumakeyer](../assets/img_out/male_lumakeyer.jpg) |
| ![tree](../assets/img_in/tree.jpg) | ![tree_lumakeyer](../assets/img_out/tree_lumakeyer.jpg) |

### Configuration

```ini
[imageFilter1]
id=ibp.imagefilter.lumakeyer
bypass=false
interpolationmode=75
inverted=1
isinverted=1
knots=75
outputmode=normal
preblurradius=7500.0

[info]
description=Apply a matte to the image based on its luminance
fileType=ibp.imagefilterlist
nFilters=1
name=Luma Keyer


```
