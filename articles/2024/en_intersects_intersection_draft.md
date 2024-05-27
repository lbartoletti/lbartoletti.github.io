# On Tolerance in GIS

Regularly, I get questions about certain "irregularities" encountered during common operations:

- Why does snapping in QGIS not always position exactly on the geometry?
- Why do calculations during overlay operations seem to lack precision?
- And why do a calculation and its inverse not always produce consistent results?

These questions reflect common concerns among GIS users, who expect strict accuracy and precision. However, the 3 R's: rigor, rigor, rigor, as one of my former bosses loved to say, are not always… rigorous on our computers.

While preparing an article on topology that I've owed Julien for several months, I was struck by what is known as the Baader-Meinhof phenomenon, or the frequency illusion: suddenly, this topic seemed to appear everywhere, from courses to online discussions. Between the reported issues and conversations with my colleagues, I decided to change my approach. Instead of continuing on the planned path, I opted to create a series of articles, exploring certain treatments, "problems," and differences in GIS. This article, divided into chapters, will be part of a series aimed at revealing the intricacies of GIS.

In the following sections, we will explore together:

- [The Observation: The calculations are not accurate](https://...)
- [Internal Functioning of QGIS and GEOS: How these tools manage data and geometric operations](https://...)
- [And Other Open Source GIS? Comparisons with GRASS and SAGA](https://...)
- [And in Databases? Comparisons of SQL Server, Oracle, and PostGIS](https://...)
- [Using Topology: Can topology save us?](https://...)
- [Alternative Approach: Exploring alternative methods such as using SFCGAL for more robust calculations](https://...)
- [How is it going with the competition?](https://...)
- [Algorithms and Code: How does it work?](https://...) This section will be optional, for those who do not want to see any code.
- [The Conclusion: How to stop overthinking and live a better life!](https://...)

Are you ready for the adventure? Let's dive into our GIS!

## The Observation: The Calculations Are Not Accurate

In our GIS, overlay operations such as intersections, unions, differences, etc., as well as snapping used by designers, are ubiquitous. These processes rely on similar calculations, simplified here for better understanding in this general overview.

### Identifying the Problem

#### Loading the Data

All the data used is available on my GitHub, and to simplify understanding and transferring these data into different GIS platforms, I will use the WKB and WKT formats. <autopromo> For those who want to know more about these formats, remember to follow or like to be informed about the next dedicated article. </autopromo>
Let's get back to the point, let's take an example with a line geometry, here closed, but it would be similar for a polygon. The geometries used are projected in the EPSG:3946 coordinate system, a projection of my beautiful region.

Example of a line geometry: `0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341`

From this line, I generate other lines that snap to the intersection/snap points with the following line: `010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341`

In QGIS, we can load these WKB - as well as EWKB, EWKT, WKT - with the very useful [QuickWKT](https://plugins.qgis.org/plugins/QuickWKT/) plugin.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/quickwkt_base.png?raw=true" alt="QuickWKT Base Example" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/quickwkt_base.png?raw=true)

This gives us:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/base_line.png?raw=true" alt="Base and line" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/base_line.png?raw=true)

#### Intersections of These Lines

We now have our basis to study this problem of precision/tolerance/intersection.

Let's calculate the intersection between these lines. For this, we will use the `native:lineintersections` tool in QGIS.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/native_lineintersections_base_line.png?raw=true" alt="Base and line intersection" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/native_lineintersections_base_line.png?raw=true)

We obtain two intersection points. Visually, the results meet our expectations, with the intersection points lying precisely on the lines.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/point_intersection_base_line.png?raw=true" alt="Base and line points intersection" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/point_intersection_base_line.png?raw=true)

When I say "precisely," even if visualized at an "absurd" scale, we still have this overlay.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/point_intersection_base_line_absurd_scale.png?raw=true" alt="Base and line points intersection, absurd scale" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/point_intersection_base_line_absurd_scale.png?raw=true)

For later, we will note their WKBs: `0101000000a899efc8c83c3e4175e5698166d55341` and `0101000000b5ebdd9e8f3c3e416bf8515379d55341`.

#### Creating Lines from These Intersections

QGIS offers different options for snapping. We will use two of them: snapping to a vertex and snapping to intersections.

##### Snapping to Intersection Points

First, we will use the snapping to intersections feature. Note, this is not snapping to the points we just generated, but to the intersections between the geometries. The intersection snap icon is marked with a cross. Snapping to a vertex is marked with a square.

In the video below, I show how I generated lines on either side of the main line at the intersection points.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/line_snap_draw.gif?raw=true" alt="Snap line QGIS intersection" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/line_snap_draw.gif?raw=true)

##### Snapping to Intersections

I repeat the operation, this time focusing on the intersection points. The goal is to ensure precise snapping to the main line, regardless of variations in adjacent vertices.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/line_intersection_draw.gif?raw=true" alt="Snap line QGIS intersection" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/line_intersection_draw.gif?raw=true)

##### Comparison of Snapping Points

We trust QGIS and, visually, the lines appear to be well snapped to the base line.

Now let's compare our textual geometries. We have 8 lines/segments in both cases. We should have the same starting points.

**line_intersection:**
| wkb_geom | wkt_geom |
|-|-|
| b'010200000002000000b5ebdd9e8f3c3e416bf8515379d5534124c410b4923c3e411668a7ae7bd55341' | LineString (1981583.62057374161668122 5199333.30187807511538267, 1981586.70338083151727915 5199342.7289676871150732) | 
| b'010200000002000000b5ebdd9e8f3c3e416bf8515379d553414819cb1c8c3c3e41d19efa7177d55341' | LineString (1981583.62057374161668122 5199333.30187807511538267, 1981580.11247404105961323 5199325.78092165384441614) | 
| b'010200000002000000a899efc8c83c3e4175e5698166d5534120d8f49bcc3c3e41ff84f6ed68d55341' | LineString (1981640.78490600921213627 5199258.02208839822560549, 1981644.60920477658510208 5199267.71817135717719793) | 
| b'010200000002000000a899efc8c83c3e4175e5698166d55341bc8cd13bc53c3e411d15f16064d55341' | LineString (1981640.78490600921213627 5199258.02208839822560549, 1981637.23366622533649206 5199249.5147145064547658) | 
| b'010200000002000000a899efc8c83c3e4175e5698166d5534199299383d33c3e412d56acf265d55341' | LineString (1981640.78490600921213627 5199258.02208839822560549, 1981651.51396427140571177 5199255.79176859278231859) | 
| b'010200000002000000a899efc8c83c3e4175e5698166d55341faacc621bc3c3e4126b2ed1567d55341' | LineString (1981640.78490600921213627 5199258.02208839822560549, 1981628.13193780044093728 5199260.34263280592858791) | 
| b'010200000002000000b5ebdd9e8f3c3e416bf8515379d55341dc272032983c3e41f6e5308b78d55341' | LineString (1981583.62057374161668122 5199333.30187807511538267, 1981592.19580315705388784 5199330.17485951445996761) | 
| b'010200000002000000b5ebdd9e8f3c3e416bf8515379d5534167a9c58f873c3e41c2567db879d55341' | LineString (1981583.62057374161668122 5199333.30187807511538267, 1981575.56160982861183584 5199334.88265007920563221) |

**line_snap:**
| wkb_geom | wkt_geom |
|-|-|
| b'010200000002000000b5ebdd9e8f3c3e416bf8515379d553419c2333eb913c3e417ba04d457cd55341' | LineString (1981583.62057374161668122 5199333.30187807511538267, 1981585.91874907072633505 5199345.08286296855658293) | 
| b'010200000002000000b5ebdd9e8f3c3e416bf8515379d553416d6001368d3c3e41080dad2b77d55341' | LineString (1981583.62057374161668122 5199333.30187807511538267, 1981581.2109585062135011 5199324.68243718892335892) | 
| b'010200000002000000b5ebdd9e8f3c3e416bf8515379d553412f400c50963c3e413ab69fef78d55341' | LineString (1981583.62057374161668122 5199333.30187807511538267, 1981590.31268693110905588 5199331.74412303604185581) | 
| b'010200000002000000b5ebdd9e8f3c3e416bf8515379d5534142628f76863c3e41b7d3bff479d55341' | LineString (1981583.62057374161668122 5199333.30187807511538267, 1981574.46312536345794797 5199335.82420819159597158) | 
| b'010200000002000000a899efc8c83c3e4175e5698166d55341259db491ca3c3e410908b4b168d55341' | LineString (1981640.78490600921213627 5199258.02208839822560549, 1981642.56916219857521355 5199266.77661324385553598) | 
| b'010200000002000000a899efc8c83c3e4175e5698166d5534175e25c6ad23c3e4164c45eac65d55341' | LineString (1981640.78490600921213627 5199258.02208839822560549, 1981650.41547980648465455 5199254.69328412786126137) | 
| b'010200000002000000a899efc8c83c3e4175e5698166d55341cccd8ccdc63c3e414a00e65664d55341' | LineString (1981640.78490600921213627 5199258.02208839822560549, 1981638.80292974691838026 5199249.35778815485537052) | 
| b'010200000002000000a899efc8c83c3e4175e5698166d55341d0a0d012bd3c3e4126b2ed1567d55341' | LineString (1981640.78490600921213627 5199258.02208839822560549, 1981629.07349591329693794 5199260.34263280592858791) |

We notice that our origin points in both cases are:

- `1981583.62057374161668122 5199333.30187807511538267`
- `1981640.78490600921213627 5199258.02208839822560549`

respectively in WKB:
- `0101000000b5ebdd9e8f3c3e416bf8515379d55341` 
- `0101000000a899efc8c83c3e4175e5698166d55341`

The lines therefore seem to have been correctly snapped, as shown by our WKT and WKB representations.

Indeed, we find the original coordinates `b5ebdd9e8f3c3e416bf8515379d55341` and `a899efc8c83c3e4175e5698166d55341` in the points.

#### Selection of Lines Intersecting the Base

Consequently, we have lines that are at the intersection point. If we want to verify the snapping, we use the `intersects` predicate in QGIS. To check this, let's use the "Select by Location" tool:

##### The `intersects` Predicate with `line_snap`

Let's look at `line_snap` with the base. In the examples below, even though I should only use `touches` or `intersects`, I will check everything except `disjoint`.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_line_snap_intersects_base.png?raw=true" alt="Line snap intersects base" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_line_snap_intersects_base.png?raw=true)

The result:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/selected_line_snap.png?raw=true" alt="Select line snap intersects base" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/selected_line_snap.png?raw=true)

Ouch, only 2 out of 4…

##### The `intersects` Predicate with `line_intersection`

Let's do the same for `line_intersection`:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_line_intersection_intersects_base.png?raw=true" alt="Line snap intersects base" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_line_intersection_intersects_base.png?raw=true)

And here's the result:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/selected_line_snap.png?raw=true" alt="Select line snap intersects base" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/selected_line_snap.png?raw=true)

Again, ouch. Only 2 out of 4, and from the same side. At least there is some consistency in our inconsistency…

##### Is It Only for These Two Points?

To go further, I conduct random tests by trying to snap the main line at different points. This is the `test_line` layer in the GeoPackage.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/test_line_spider.png?raw=true" alt="Spiderman" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/test_line_spider.png?raw=true)

In our OSGeo franchise, QGIS is one of our superheroes, like [Spider-Man](https://www.youtube.com/watch?v=i5P8lrgBtcU). Our little spider web lacks [glue](https://www.youtube.com/watch?v=rf6Yv4lMhhs), because our snapping is still not good:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/test_line_spider_selected.png?raw=true" alt="Spider selected" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/test_line_spider_selected.png?raw=true)

##### Testing Every Meter on the Base

A few years ago, I developed a tool to create transects. It generates strips at a given angle, primarily used for cross-profile analyses in civil engineering, ecology, etc. Here, we will create a transect every meter on either side of the main line.

First, let's densify the geometry:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_densify.png?raw=true" alt="Densify" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_densify.png?raw=true)

Next, we generate transects from this layer to the left and right of the line:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_transect_densify_left.png?raw=true" alt="Transect densify left" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_transect_densify_left.png?raw=true) [<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_transect_densify_right.png?raw=true" alt="Transect densify right" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_transect_densify_right.png?raw=true)

Our drawing looks like this:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/transect_left_right.png?raw=true" alt="Canvas transect" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/transect_left_right.png?raw=true)

Now, let's select the features from `transect_left` and `transect_right` that intersect the base:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_select_transect_left.png?raw=true" alt="Select transect left" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_select_transect_left.png?raw=true) [<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_select_transect_right.png?raw=true" alt="Select transect right" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_select_transect_right.png?raw=true)

And no, it's not the transect algorithm that is all buggy.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/40c1b2bf53b7842b3a86eb009525578fe492efd8/assets/2024_intersection_intersects/selected_transect_left_right.png?raw=true" alt="Canvas select transect" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/40c1b2bf53b7842b3a86eb009525578fe492efd8/assets/2024_intersection_intersects/selected_transect_left_right.png?raw=true)

But, as they say… "Caramba! Another failure!"

![carambar](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/carramba.jpg?raw=true)

What if I test on the densified geometry?

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_select_transect_left_densify.png?raw=true" alt="Select transect densify left" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_select_transect_left_densify.png?raw=true)
[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_select_transect_right_densify.png?raw=true" alt="Select transect densify right" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fr_select_transect_right_densify.png?raw=true)

And our result?

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/selected_transect_left_right_densify.png?raw=true" alt="Canvas select transect densify" width="348" height="420">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/selected_transect_left_right_densify.png?raw=true)

It works! But why?

Well, we will see that later.

## GEOS at the Heart of QGIS

In the previous section, we identified the problem: the result of an intersection does not always intersect the original data. This reality can surprise new GIS users and frustrate experienced ones who seek precision in their spatial analyses.

In this section, we will dive into the inner workings of GIS by exploring how these processes operate. We will focus particularly on the role of GEOS in QGIS.

### What is GEOS?

GEOS (Geometry Engine - Open Source) is a C++ library that provides computational geometry functions for OGC Simple Feature geometries. It is widely used in various GIS tools, including QGIS, to perform geometric calculations. GEOS is an implementation of the JTS (Java Topology Suite) API, designed to manipulate 2D planar geometries.

![GEOS diagram from crunchy data](https://f.hubspotusercontent00.net/hubfs/2283855/geos-jts%20(1).png)
(Source: https://www.crunchydata.com/blog/performance-improvements-in-geos)

### The Role of GEOS in QGIS

In QGIS, GEOS plays a crucial role in processing geographic data. It is particularly used to evaluate spatial predicates such as `intersects`, `touches`, `disjoint`, etc. These predicates are essential for determining spatial relationships between different geometries.

For those familiar with QGIS's code, it is true that some processes are not performed by GEOS but by QGIS itself. We will look into this in the section on algorithm analysis, but essentially, it doesn't change much regarding the problem.

### Using GEOS Without QGIS

If you didn't know, it is also possible to perform calculations directly with GEOS, without using QGIS's graphical interface. One way to do this is by using `geosop`, a command-line tool that allows you to manipulate geometries with GEOS functions.

`geosop` allows users to execute complex operations on geometries through simple commands. For example, to check if one geometry intersects another, you can use the following command:

```shell
> geosop -a "LineString(0 0, 10 10)" -b "Point(5 5)" intersects
true
> geosop -a "LineString(0 0, 10 10)" -b "Point(4 4)" intersects
false
```

Here, we used the WKT format to test if geometries `a` and `b` intersect. Later, we will add and explain other options as we use them.

### Testing Our Case Directly with GEOS

As a reminder, our geometries are as follows:

- base: `0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341`
- line: `010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341`

To get hands-on experience, let's test if our geometries indeed intersect. We haven't tested this in QGIS, but it seems obvious.

```shell
> geosop \
-a \
"0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341" \
-b \
"010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341" \
intersects
```

Here, I used `\` to split the command into multiple lines for readability; you can, of course, run it all on a single line.

The returned result is `true`. This is consistent.

Now, let's calculate the intersection. To do this, we use the `intersection` operator instead of `intersects`, nothing complicated.

```shell
> geosop \
-a \
"0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341" \
-b \
"010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341" \
intersection
```

The response is `MULTIPOINT ((1981640.7849060092 5199258.022088398), (1981583.6205737416 5199333.301878075))`

Ah! This is a small difference from QGIS, which returns two points. Here, GEOS returns a MULTIPOINT, which, in my opinion, is more consistent, but never mind.
WKT is more readable, but it has the drawback of not always having the same representation. QGIS returns 17 decimals, while GEOS returns 10; which, in any case, is already too many for a projected coordinate system. We will discuss this later.

To avoid these differences, we will work with WKB. To retrieve it, simply add the WKB option to the `-f` output format option:

```shell
> geosop \
-a \
"0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341" \
-b \
"010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341" \
-f wkb \
intersection
```

Our result is: `0104000000020000000101000000A899EFC8C83C3E4175E5698166D553410101000000B5EBDD9E8F3C3E416BF8515379D55341`

You will understand the upcoming operation better after reading the article on WKT and WKB. In the meantime, I ask you to trust me.
Here, we have the representation of a multipoint, where the coordinates can be found in these parts:

- `0101000000A899EFC8C83C3E4175E5698166D55341`
- `0101000000B5EBDD9E8F3C3E416BF8515379D55341`

Which, considering the insignificant difference between upper and lower case, is equivalent to:

- `0101000000a899efc8c83c3e4175e5698166d55341`
- `0101000000b5ebdd9e8f3c3e416bf8515379d55341`

Great. Does the point intersect or touch any of the original geometries?

```shell
> geosop \
-a \
"0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341" \
-b \
"0104000000020000000101000000A899EFC8C83C3E4175E5698166D553410101000000B5EBDD9E8F3C3E416BF8515379D55341" \
intersects
```

Here, I test if the multipoint intersects the `base` geometry. This returns false.

The same goes for the multipoint and `line`:

```shell
> geosop \
-a \
"010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341" \
-b \
"0104000000020000000101000000A899EFC8C83C3E4175E5698166D553410101000000B5EBDD9E8F3C3E416BF

8515379D55341" \
intersects
```

You can also try directly with the points `0101000000A899EFC8C83C3E4175E5698166D55341` and `0101000000B5EBDD9E8F3C3E416BF8515379D55341` instead of the multipoint.
Similarly, you can test other predicates like `touches`, the result will always be `false`...
Except for... `disjoint`, which means the points are not on the geometries.

So why, if the points are not on the lines, did we have segments in QGIS that intersected the original geometry?

If you pay close attention, you may notice that only one side of the two had an intersection; and it wasn't always the same side.
I'll let you look at the images from the previous section.

We are starting to get to the heart of the problem. The intersection point is on one side or the other of the segment. If we were at a hyper zoom, we would have something like this:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/example_grid_line.png?raw=true" alt="Example points along line" width="349" height="203">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/example_grid_line.png?raw=true)

In simple terms, we could say that the points are on tiny grids. The point is not on the line, but very close.

By the way, let's ask GEOS where the points are relative to our original geometries.

The distance of the first point from the `base`:

```shell
> geosop \
-a \
"0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341" \
-b \
"0101000000A899EFC8C83C3E4175E5698166D55341" \
distance
```

`3.356426828584339e-10`

The second point:

```shell
> geosop \
-a \
"0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341" \
-b \
"0101000000B5EBDD9E8F3C3E416BF8515379D55341" \
distance
```

`3.0508414550559678e-10`

The distance of the first point from `line`:

```shell
> geosop \
-a \
"010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341" \
-b \
"0101000000A899EFC8C83C3E4175E5698166D55341" \
distance
```

`1.812697635977354e-10`

The second point:

```shell
> geosop \
-a \
"010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341" \
-b \
"0101000000B5EBDD9E8F3C3E416BF8515379D55341" \
distance
```

`1.7234470954287178e-10`

We notice that the result is not 0, but very close. It's roughly 0, but there's a "joke" around 10 digits after the decimal point.
For those interested, I will come back to the importance of calculation in the algorithm section.
In the meantime, we observe that QGIS gives the same result as GEOS. This is not surprising since behind QGIS [^1], it's GEOS.

So, is GEOS wrong? No, GEOS gives the "correct" result, but the truth lies elsewhere.
We will continue this exploration in the following sections.

[^1]: As explained earlier, QGIS performs some calculations identical to those of GEOS but without using this library. In particular, snapping does not rely on GEOS but on equivalent calculations. I simplify here to avoid losing the less knowledgeable readers of this ecosystem.

## And Other Open Source GIS? Comparisons with GRASS and SAGA

### GRASS: The Venerable GIS

Since I've already revealed some internal secrets, Julien ~~asked~~ suggested I make an updated version of this [article](https://geotribu.fr/articles/2014/2014-11-13_corriger_automatiquement_geometries_invalides_qgis/). I ~~procrastinated~~ deliberately waited until 2024 to celebrate the article's 10th anniversary. But, I won't do that here. Nonetheless, we will certainly use GRASS to see if our case differs with GRASS.

For our experiment with GRASS, some manipulations are necessary as we cannot directly perform our calculations with WKB. To simplify reproducibility for readers, I've added processing models to the project; they are also available individually on my GitHub.

#### Using v.overlay

In GRASS, `v.overlay` allows you to perform overlay operations (intersection, union, difference) between two vector layers. It requires two vectors, and the second one, B, must be of type "area" (polygon in OGC terminology). If the vector is not a polygon, it needs to be converted before processing; this is our case.

The `base` layer is a closed polyline and will be used for conversion into a polygon. Purists will check that the WKB coordinates are identical between the linestring and the (multi)polygon. There are several ways to do this, but to make it accessible to everyone, we will use GRASS via QGIS for the initial conversions; then we will use only GRASS tools.

![grass_line_overlay_points](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/data/processing/grass_line_overlay_points.svg)

Our algorithm will convert the `base` into a polygon and then perform the overlay operation. To calculate the intersection between `line` and `base_poly`, we extract the intersection points which can be displayed in QGIS. We get the same result, and here, I skip the details, but we find our WKBs:

- `0101000000a899efc8c83c3e4175e5698166d55341`
- `0101000000b5ebdd9e8f3c3e416bf8515379d55341`

For the `intersects` predicate, we will continue using GRASS with the `v.select` command and the `intersects` operation. 
Again false... at least, the points returned are the endpoints of `line` and no points for `base` or `base_poly`.

![grass_select_line_overlay](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/data/processing/grass_select_line_overlay_points.svg?raw=true)

The model is available [here](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/data/processing/line_overlay_points.model3).

This is at least reassuring, because behind `v.select` with the `intersects` operator, it uses… GEOS.
There is another way to perform the selection, but I will save it for the next part :wink:

### SAGA: The Forgotten Knight of GIS

If [SAGA](https://fr.wikipedia.org/wiki/Saga_(personnage)) is the most powerful knight of the zodiac, [SAGA GIS](https://saga-gis.sourceforge.io/en/index.html) is unfortunately often the forgotten knight of GIS. It has some processes that are not available in QGIS and can also be useful for others that do exist.

In recent versions, it is no longer possible to have SAGA processes directly from QGIS. You need to install the [SAGA NG](link to the plugin) plugin, but it has some limitations preventing me from using it for this article.
For this part, I will use the SAGA interface directly, mainly to visualize the result.

It is interesting to compare SAGA's results with GEOS/QGIS. Indeed, overlay operations do not rely on GEOS but on another dedicated library; for those interested, I will discuss this in the algorithm section.

As with the others, we will determine the intersection points between `line` and `base`, then test if they intersect the original geometries and also, how many lines from `test_line` intersect `base`.

First of all, SAGA cannot read GeoPackage files. I converted our layers into the good old ShapeFile format.

We load these files into the SAGA interface.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_load_shapes.png?raw=true" alt="Load shapes" width="349" height="203">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_load_shapes.png?raw=true)

We display our data. Nothing surprising, we see our two geometries.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_map_base_line.png?raw=true" alt="Map base line" width="349" height="203">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_map_base_line.png?raw=true)

First step, verify the intersection calculation. In SAGA terminology, the intersection between lines is called "Crossing".
We run the process: Geoprocessing -> Shapes -> Lines -> Line Crossing

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_line_crossing.png?raw=true" alt="Line Crossing" width="349" height="203">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_line_crossing.png?raw=true)

We find our two points:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_map_crossing.png?raw=true" alt="Map crossing" width="349" height="203">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_map_crossing.png?raw=true)

I skip the steps for retrieving them, but the WKBs are the same:

- `0101000000b5ebdd9e8f3c3e416bf8515379d55341`
- `0101000000a899efc8c83c3e4175e5698166d55341`

SAGA's algorithm, which does not use GEOS, returns the same result. Very good!

### Selection

Now, let's check if the points intersect our base. For this, we use the "Selection by Location" tool:
Geoprocessing -> Shapes -> Selection -> Selection by Location.

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_select_crossing_base.png?raw=true" alt="Select crossing base" width="349" height="203">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_select_crossing_base.png?raw=true)

Bang, interesting error. This only works for a point with a polygon!

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_select_crossing_base_error.png?raw=true" alt="Select crossing base error" width="349" height="203">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/202

4_intersection_intersects/images/saga_select_crossing_base_error.png?raw=true)

In our experiment with GRASS, we had a similar problem. We will therefore test with `base_poly`:

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_select_crossing_base_poly.png?raw=true" alt="Select crossing base poly" width="349" height="203">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_select_crossing_base_poly.png?raw=true)

No selection. The execution message clearly states (in English): "selected shapes: 0"

Even though one might criticize the method since the drawing was done in another context, QGIS/GEOS, we will still test the line selection:

Let's take our example between `base` and `test_line`.

20 out of 34, just like GEOS!

[<img src="https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_select_base_test_line.png?raw=true" alt="Select base test line" width="349" height="203">](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/saga_select_base_test_line.png?raw=true)

In a way, this is normal. However, the first test with crossing also shows us that the intersection point does not intersect the original geometry, just like with GEOS.


## And in Databases? Comparisons of SQL Server, Oracle, and PostGIS

If you've been following the previous sections, you know that PostGIS uses GEOS to perform most of its operations. Specifically, our case involving intersection and the `intersects` predicate will be delegated to GEOS.

First, we will verify that the results from PostGIS are identical to those from "native" GEOS, and then we will compare them with other proprietary databases.

While I am very familiar with PostGIS, my knowledge of other databases is somewhat limited. I apologize if there are any errors in the queries or if there are more academic ways to perform certain tasks. If you find any mistakes or know of better methods, please feel free to leave a respectful comment, and I will correct them.

### PostGIS: Same Result as GEOS

The title gives away the conclusion, but it was already anticipated.

We will use our two WKB geometries:

- base: `0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341`
- line: `010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341`

```sql
SELECT ST_Intersection(base, line)
FROM
    ST_GeomFromWKB(decode('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341', 'hex'), 3946) as base,
    ST_GeomFromWKB(decode('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341', 'hex'), 3946) as line;
```

The result is an EWKB, hence the difference from our GEOS WKB; I will explain this in the next article on WKB/WKT.

By removing the "E" part, i.e., the SRID `0206A0F0`, we get the "correct" WKB:

`0104000000020000000101000000A899EFC8C83C3E4175E5698166D553410101000000B5EBDD9E8F3C3E416BF8515379D55341`

We can directly retrieve this with PostGIS using `ST_AsBinary`:

```sql
SELECT ST_AsBinary(ST_Intersection(base, line))
FROM
    ST_GeomFromWKB(decode('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341', 'hex'), 3946) as base,
    ST_GeomFromWKB(decode('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341', 'hex'), 3946) as line;
```

`\x0104000000020000000101000000a899efc8c83c3e4175e5698166d553410101000000b5ebdd9e8f3c3e416bf8515379d55341`

For readable geometry using the textual format, we use `ST_AsText`:

```sql
SELECT ST_AsText(ST_Intersection(base, line))
FROM
    ST_GeomFromWKB(decode('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341', 'hex'), 3946) as base,
    ST_GeomFromWKB(decode('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341', 'hex'), 3946) as line;
```

`MULTIPOINT((1981640.7849060092 5199258.022088398),(1981583.6205737416 5199333.301878075))`

I will slightly adapt the query, using CTEs, for better readability later on.

```sql
WITH
base AS (
    SELECT ST_GeomFromWKB(decode('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341', 'hex'), 3946) AS geom
),
line AS (
    SELECT ST_GeomFromWKB(decode('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341', 'hex'), 3946) AS geom
)
SELECT
    ST_AsBinary(ST_Intersection(base.geom, line.geom)),
    ST_AsText(ST_Intersection(base.geom, line.geom))
FROM base, line;
```

The same result is returned:
`\x0104000000020000000101000000a899efc8c83c3e4175e5698166d553410101000000b5ebdd9e8f3c3e416bf8515379d55341`
`MULTIPOINT((1981640.7849060092 5199258.022088398),(1981583.6205737416 5199333.301878075))`

```sql
WITH
base AS (
    SELECT ST_GeomFromWKB(decode('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341', 'hex'), 3946) AS geom
),
line AS (
    SELECT ST_GeomFromWKB(decode('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341', 'hex'), 3946) AS geom
)
SELECT
    ST_Intersects(base.geom, ST_Intersection(base.geom, line.geom)),
    ST_Intersects(line.geom, ST_Intersection(base.geom, line.geom)),
    ST_Distance(base.geom, ST_Intersection(base.geom, line.geom)),
    ST_Distance(line.geom, ST_Intersection(base.geom, line.geom))
FROM base, line;
```

Again, we get our unfortunate `false` and our distance very close to 0, but not 0.

What's happening with the others?

### Microsoft SQL Server

The SQL Server syntax is different from PostGIS, but still quite readable. The main difference is the addition of "0x" in the input WKB. This represents its hexadecimal format; more information on this will be covered in the next article.

Here's the query:

```sql
WITH
base AS (
    SELECT geometry::STGeomFromWKB(0x0102000000050000007997C6B68D3C3E4139EB62C260D55341AC9EA7316A3C3E41CBEB40E073D55341403E0BFBC33C3E41B3FC06F380D55341387A2A800C3D3E41F256B8176DD553417997C6B68D3C3E4139EB62C260D55341, 3946) AS geom
),
line AS (
    SELECT geometry::STGeomFromWKB(0x010200000002000000EA9C6D2B873C3E41A03D941B7CD5534133DB7796CE3C3E413FBA569864D55341, 3946) AS geom
)
SELECT
    base.geom.STIntersection(line.geom) AS WKB,
    base.geom.STIntersects(base.geom.STIntersection(line.geom)) AS Intersects_Base_Line,
    line.geom.STIntersects(base.geom.STIntersection(line.geom)) AS Intersects_Line_Base,
    base.geom.STDistance(base.geom.STIntersection(line.geom)) AS Distance_Base_Line,
    line.geom.STDistance(base.geom.STIntersection(line.geom)) AS Distance_Line_Base
FROM base, line;
```

The result, on SQL Server 15.0.4153, formatted, is:

```
0x6A0F0000010402000000B5EBDD9E8F3C3E416BF8515379D55341A899EFC8C83C3E4175E5698166D55341020000000100000000010100000003000000FFFFFFFF0000000004000000000000000001000000000100000001,
false,false,
0,0.00000000023283064365386963
```

The result of `intersects` is false, yet for one case, the distance is equal to 0. Interesting—is it really zero or just so close to zero that it returns 0? The other distance matches that of GEOS: `2.3283064365386963e-10`.

For the WKB, it is "particular," but we find our coordinates:

- `A899EFC8C83C3E4175E5698166D55341`
- `B5EBDD9E8F3C3E416BF8515379D55341`

It doesn't seem, and I'm almost certain, that SQL Server uses GEOS. Again, the problem is not in GEOS's code.

### ORACLE Spatial

Oracle will give us some interesting insights. Let's go directly to the query:

```sql
WITH
base AS (
    SELECT SDO_UTIL.FROM_WKBGEOMETRY(
        HEXTORAW('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341')
    ) AS geom
    FROM DUAL
),
line AS (
    SELECT SDO_UTIL.FROM_WKBGEOMETRY(
        HEXTORAW('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341')
    ) AS geom
    FROM DUAL
)
SELECT
    SDO_GEOM.SDO_INTERSECTION(base.geom, line.geom) AS Intersection,
    SDO_UTIL.TO_WKTGEOMETRY(SDO_GEOM.SDO_INTERSECTION(base.geom, line.geom)) AS WKT,
    SDO_UTIL.TO_WKBGEOMETRY(SDO_GEOM.SDO_INTERSECTION(base.geom, line.geom)) AS WKB,
    SDO_GEOM.RELATE(base.geom, 'ANYINTERACT', SDO_GEOM.SDO_INTERSECTION(base.geom, line.geom), 0.00000000001) AS Intersects_Base_Line,
    SDO_GEOM.RELATE(line.geom, 'ANYINTERACT', SDO_GEOM.SDO_INTERSECTION(base.geom, line.geom), 0.00000000001) AS Intersects_Line_Base,
    SDO_GEOM.SDO_DISTANCE(base.geom, SDO_GEOM.SDO_INTERSECTION(base.geom, line.geom), 0.00000000001) AS Distance_Base_Line,
    SDO_GEOM.SDO_DISTANCE(line.geom, SDO_GEOM.SDO_INTERSECTION(base.geom, line.geom), 0.00000000001) AS Distance_Line_Base
FROM base, line;
```

The result, on ORACLE XE 21, is:

```
"{2005,null,null,{1,1,2},{1981583.62057374,5199333.30187808,1981640.78490601,5199258.0220884}}",
"MULTIPOINT ((1981583.62057374 5199333.30187808), (1981640.78490601 5199258.0220884))",
0x0000000004000000020000000001413E3C8F9EDDEBAE4153D5795351F8700000000001413E3CC8C8EF99AB4153D5668169E577,
FALSE,FALSE,
0.00000000104125029291017,0.00000000023283064365387
```

Once again, here, a `false` result. Like PostGIS and SQL Server, we find our distance of about `2.3e-10`, and another of `1e-9`. I find it interesting to have this small difference in one of the distances, but I digress.

Here, I've adapted the query to the SDO syntax. Let's explain the query and the result:

As with SQL Server, we need to convert the hexadecimal WKB for Oracle, using `HEXTORAW`.

If PostGIS returns an EWKB for a geometry result, Oracle returns its internal representation:
`{2005,null,null,{1,1,2},{1981583.62057374,5199333.30187808,1981640.78490601,5199258.0220884}}`

What interests us here is the code `2005`, which means MultiPoint 2D, as well as the coordinate array X/Y `{1981583.62057374,5199333.30187808,1981640.78490601,5199258.0220884}`.

We find this information with the WKT representation we are used to:
`MULTIPOINT ((1981583.62057374 5199333.30187808), (1981640.784906015199258.0220884))`

I won't elaborate on the WKB, which is "strange." It is in Big Endian, whereas until now, I have only seen Little Endian; again, more explanations in the article on WKB/WKT. However, we have some differences between them. Likely due to the precision of the result; not being an Oracle expert, I lack some understanding and need further testing.

However, at a glance, we have the same result:

| Base   | x1              | y1              | x2              | y2              |
|--------|-----------------|-----------------|-----------------|-----------------|
| ORACLE | 1981583.62057374 | 5199333.30187808 | 1981640.78490601 | 5199258.0220884 |
| PostGIS | 1981583.6205737416 | 5199333.301878075 | 1981640.7849060092 | 5199258.022088398 |

I mentioned in the introduction that Oracle [would] give us interesting insights, but so far, it seems like the others? Yes, but, there's a parameter I haven't explained yet. Where does `0.00000000001` come from?

In my version, I don't have `ST_Intersects` or `SDO_Intersects`; I have to use [`RELATE`](https://docs.oracle.com/en/database/oracle/oracle-database/21/spatl/SDO_GEOM-reference.html#GUID-E1209A71-F5D8-42A9-A93E-72657B115579). We also have this with [PostGIS](https://postgis.net/docs/ST_Relate.html) (and GEOS). `ANYINTERACT` returns `TRUE` if the objects are not disjoint, which is what we want.
So, we have our equivalent of `ST_Intersects`, or more precisely, `not ST_Disjoint`. However, this still doesn't explain what `0.00000000001` is. Do you remember the main title of this series? [Tolerance](https://docs.oracle.com/en/database/oracle/oracle-database/21/spatl/spatial-concepts.html#GUID-7469388B-6D23-4294-904F-78

CA3B7191D3).

It is a tolerance in the predicate calculation. With an "extreme" value, like here, the result is false. However, if we use a value more consistent with our unit, such as `1e-6`, we finally get our long-awaited "correct" result:

```
"{2005,null,null,{1,1,2},{1981583.62057374,5199333.30187808,1981640.78490601,5199258.0220884}}",
"MULTIPOINT ((1981583.62057374 5199333.30187808), (1981640.78490601 5199258.0220884))",
0x0000000004000000020000000001413E3C8F9EDDEBAE4153D5795351F8700000000001413E3CC8C8EF99AB4153D5668169E577,
TRUE,TRUE,
0,0
```

The WKT and WKB geometries are identical, but we get `TRUE` and `0`.

How to interpret this? Using tolerance makes the calculation more... tolerant. In our case, it returns true if the point is within approximately the given tolerance of the geometry, here `1e-6`.

In PostGIS, we could rewrite this with `ST_DWithin`:

```sql
WITH 
base AS (
    SELECT ST_GeomFromWKB(decode('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341', 'hex'), 3946) AS geom
),
line AS (
    SELECT ST_GeomFromWKB(decode('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341', 'hex'), 3946) AS geom
)
SELECT 
    ST_DWithin(base.geom, ST_Intersection(base.geom,line.geom), 1e-6), ST_DWithin(line.geom, ST_Intersection(base.geom,line.geom), 1e-6)
FROM base, line;
```

Finally, regarding the distance, Oracle accepts a tolerance parameter and gives a different result depending on this parameter. One might think that the distance should always be the same. However, I believe—speculation, as I am not well-versed in Oracle—that it serves to round if within its range, and then, for our case, returns 0, rather than almost zero.

Our exploration is not yet complete, though we are getting closer to an explanation. We have seen that, like the OpenSource GIS, we cannot correctly return the `intersects` predicate of an intersection unless we are tolerant, and we will revisit this.

## Using Topology: Can Topology Save Us?

In GIS, we often distinguish between two types of models for representing spatial data: the "spaghetti" model and the topological model.

**Spaghetti GIS**: In a spaghetti system, geographic entities are stored and managed individually, without explicit relationships between them. Each line or polygon is drawn without considering other elements that might touch or overlap. This can lead to inconsistencies, such as duplicate lines or unmanaged intersections, complicating spatial analyses and potentially reducing result accuracy.

**Topology**: In contrast, topology in QGIS (or any other GIS supporting this model) ensures that spatial entities are stored with rules defining and maintaining the spatial relationships between entities. For example, two adjacent polygons will share a common line without duplication, and intersections will be managed correctly. Topological management helps prevent geometric errors, improves the accuracy of analyses, and facilitates data maintenance.

No, this is still not the article where I will cover topology in detail. At best, consider this a teaser.

### Revisiting GRASS

One of the characteristics of GRASS GIS is its topological management of vector data. Unlike other systems that use a "spaghetti" data model, where geometric entities are stored without explicit consideration of their spatial relationships, GRASS GIS maintains a strict topological structure.

#### Overlap for Better Selection in GRASS

In the previous section, I mentioned there was another way to use `v.select`. Indeed, we used GEOS with `intersects`.

The GRASS documentation mentions another operator, `overlap`, which is the default and uses native GRASS functions. What's the result?

Yes, the expected one! Finally! For both the `base` and the `line`, the intersection points do indeed intersect the original geometries!

#### Using v.clean

But, since GRASS is topological, we can also use it to find our intersections. To do this, we'll perform a little hack to simplify our calculations.

To use topology, we will merge our two lines from `base` and `line` and then clean them.
The cleaning, via `v.clean`, will be done using only the `break` tool.
We will go from:
< insert image >
to
< insert image with break >

Now, let's see if our `v.select` function with `intersects` works.

Hooray!

What are the points? The same as before. But, what happened? Well, let's go back to QGIS and explain this graphically.

### Topology in QGIS

In QGIS, there is an editing mode that I particularly like, which is… topological editing.

QGIS is spaghetti. All the data is in this pasta dish: geographic entities are stored individually without explicit topological relationships. But, although QGIS uses a "spaghetti" data model, the software offers tools that help maintain geometric consistency between layers.

In particular, here, we will use the topological editing function, which, when snapping to a segment, will add nodes to the snapped segment.

< insert gif >

The `base_topology` layer is a copy of `base` on which I drew, using topological editing, the `test_line` layer.

If we redo our "select by location" test with the `intersects` predicate, we have our 34 lines selected.

< insert image >

To be sure it's not just vertices on the other side of the line, we can extract them and redo the operation.

< insert image >

Topology is great, topology is good, it will save our calculations!

### The Limits of Topology

Yes, topology is excellent, and its use in QGIS, through its tools or via GRASS, is very powerful. However, there are some drawbacks:

- More difficult to use/maintain;
- Longer processing times when integrating non-topological external data;
- More significant vertex counts;
- Modification of the original data;
- etc.

In particular, I will soon explain what I mean by "modification of the original data."

In the first section, I mentioned that the distance from the point to the original geometry was close to zero but not exactly zero.

With topological editing in QGIS or GRASS storage, the intersection points coincide with the vertices of our geometries, thanks to topology.

However, besides the added nodes, is our geometry the same? Visually, once again, apart from the nodes, it seems identical.

Let's compare the angles of the segments.

In our original geometry, we have four segments, with azimuths, in radians:

![](https://github.com/lbartoletti/lbartoletti.github.io/blob/e2a7c896516af05da86d079c589edda415471e83/assets/2024_intersection_intersects/data/processing/qgis_segments_azimuth.svg?raw=true)

Starting from the bottom left and rotating clockwise:
- 4.341309933406307
- 1.0434054052601267
- 2.401617769614756
- 5.848325554504713

In our topologically edited base?

On the first original segment:
- 4.341309933414985
- 4.3413099333929175
- 4.341309933355925
- 4.341309933411935
- 4.34130993343072
- 4.341309933414806
- 4.341309933388343
- 4.341309933497785
- 4.341309933391953

The second:
- 5.848325554496416
- 5.848325554486275
- 5.8483255545512725
- 5.848325554512849
- 5.848325554487155
- 5.848325554478932
- 5.848325554530697
- 5.848325554523524
- 5.848325554479168

The third:
- 1.0434054052235737
- 1.043405405251191
- 1.0434054052730521
- 1.0434054052793966
- 1.0434054052156936
- 1.0434054052777524
- 1.043405405257653
- 1.0434054052902961
- 1.0434054052482782

The fourth:
- 2.401617769621854
- 2.401617769590059
- 2.4016177695994174
- 2.4016177696075154
- 2.401617769638797
- 2.401617769583275
- 2.4016177696211147
- 2.401617769630777
- 2.4016177696067103
- 2.4016177696313

The new segments are not "aligned" like the original segment. Not by much, actually, because if we convert these angles to degrees, we get four values, identical to our original segments:
- 335.084
- 59.783
- 137.603
- 248.739

In reality, not entirely, but I deliberately rounded to three decimal places.
Why did I do that? Tired of carrying so many digits.
And, is it really necessary to have so many digits after the decimal point? :wink:


## Alternative Approach: Exploring Methods Such as Using SFCGAL for More Robust Calculations

In previous sections, we showed that the `intersects` of an intersection was false, except with topology or tolerance.

Now we will use an "alternative approach" in calculations. I promise to reserve the code part for another section with optional reading.

Let me introduce SFCGAL.

> SFCGAL is a C++ wrapper library around [CGAL](https://www.cgal.org/) aimed at supporting ISO 19107:2013 and OGC Simple Features Access 1.2 for 3D operations.
> 
> SFCGAL provides standardized geometry types and operations that can be accessed via its C or C++ APIs. PostGIS uses the C API to expose some SFCGAL functions in spatial databases (see the PostGIS manual).
> 
> The coordinates of geometries have an exact rational number representation and can be in 2D or 3D.

In short, SFCGAL performs geometry operations like we know in GIS but with the CGAL engine and, importantly, with "different" numbers: exact rational numbers. A more detailed explanation will be given in the algorithm and code section, but consider these as fractions.

We will use SFCGAL in two ways to compare their results, with Python and with PostGIS.

### Python with PySFCGAL

PySFCGAL is a Python interface to the SFCGAL library, currently under development and packaging. Without an `sfcgalop` application like `geosop`—which is under development—the Python interface allows for easier calculations than in C or C++. I promise, it's "readable" code.

On my FreeBSD system, here's how I use it for our test:

```python 
# Import the library
from pysfcgal import sfcgal
# Read the base wkb
base = sfcgal.read_wkb('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341')
# Read the line wkb
line = sfcgal.read_wkb('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341')
```

Our intersection calculation will be done simply with this line:

```python
# Calculate the intersection
result = base.intersection(line)
# Display the WKT with 10 decimals
print(result.wktDecim(10))
```

The result `MULTIPOINT((1981583.62057374 5199333.30187807),(1981640.78490601 5199258.02208840))` is consistent with what we have seen so far `MULTIPOINT ((1981583.6205737416 5199333.301878075), (1981640.7849060092 5199258.022088398))`.

We note a small difference starting from the 10th digit. This translates into the WKB, which, aside from the inversion of points, is slightly different.

```python 
print(result.wkb)
```

`0104000000020000000101000000b6ebdd9e8f3c3e416af8515379d553410101000000a899efc8c83c3e4175e5698166d55341`

And our `intersects` calculation?

```python
print(result.intersects(base))
print(result.intersects(line))
```

It's a double `TRUE`!

- You mentioned SFCGAL is available in PostGIS?
- Yes, do you want to see what that looks like?
- Well… True!
- Let's see…

### PostGIS with SFCGAL

The main consumer of SFCGAL is PostGIS, particularly for 3D calculations and certain operations not available in GEOS. 
At the time of writing, PostGIS version 3.5 is not yet released. However, it contains all the SFCGAL algorithms, including those that may overlap with GEOS.
You know your `ST_` functions in PostGIS; replace the prefix with `CG_`, and you get the SFCGAL functions.

Since it is an extension, it needs to be activated with the SQL command `CREATE EXTENSION postgis_sfcgal CASCADE;`

If we take our PostGIS query, for SFCGAL, it looks like this:

```sql
WITH
base AS (
    SELECT ST_GeomFromWKB(decode('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341', 'hex'), 3946) AS geom
),
line AS (
    SELECT ST_GeomFromWKB(decode('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341', 'hex'), 3946) AS geom
)
SELECT 
    CG_Intersects(base.geom, CG_Intersection(base.geom,line.geom)), CG_Intersects(line.geom, CG_Intersection(base.geom,line.geom)),
    CG_Distance(base.geom, CG_Intersection(base.geom,line.geom)), CG_Distance(line.geom, CG_Intersection(base.geom,line.geom))
FROM base, line
```

The long-awaited result:

```sql
 cg_intersects | cg_intersects |      cg_distance       |     cg_distance
---------------|---------------|------------------------|----------------------
 f             | f             | 3.0508500689910857e-10 | 1.72344247851053e-10
```

How can it be "false"? It was true with Python.

Indeed, but it’s important to understand that when executing this query, several conversions take place.

As mentioned earlier, SFCGAL uses "different numbers," sorts of fractions. In reality, its internal representation looks like this:

```python
print(result.wkt)
```

We get a "WKT" but with fractions:

`MULTIPOINT((39835383350819973229271557358571370825/20102802090827818983138002993152 26130292092976317370966223130398592513/5025700522706954745784500748288),(4210607705173521426482690542319302151/2124808763144847138096555229184 2761857251796143622778067803204906245/531202190786211784524138807296))`

Converting from fraction to floating-point "double" results in a slight approximation, and we don't get back to our original bits. Hence this behavior.

For the curious, the "algorithm" section will include an example in Python SFCGAL and other examples in PostGIS SFCGAL.

What’s important to understand is that SFCGAL can provide the correct result if you stay within its numbering or have numbers that are finite or within the range of double values.

There could be a way to get the correct calculation in PostGIS with SFCGAL, but it's not implemented yet.

## How Does the Competition Fare?

We won't cover all of them, but we'll look at two well-known tools often installed alongside QGIS: FME and ArcGIS Pro.

### FME

For FME, let's get straight to it. I insert the WKB, perform an intersection test, and check if the points intersect `line` and `base`.

You can find the [fmw file on my GitHub](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/data/fme_test_intersects.fmw).

And the result:

![](https://github.com/lbartoletti/lbartoletti.github.io/blob/b8c9cd77f5474d49e06c9dcd8a0bc196eb445736/assets/2024_intersection_intersects/images/fme_test_intersects.png?raw=true)

KO!

FME uses and contributes to open-source tools. However, even though the result is the same as with GEOS, I don't believe GEOS is being used here. If anyone has information on this, please leave a comment.

### ESRI ArcGIS Pro

Like with QGIS, we will test our problem in two ways: through the GUI and directly with WKB.

#### Using ShapeFile

To the best of my knowledge, ArcGIS does not support GeoPackage files. No worries, we will use the good old ShapeFile, which will be imported into a GeoDatabase.

To perform the intersection calculation, we use the [Pairwise Intersect](https://pro.arcgis.com/en/pro-app/latest/tool-reference/analysis/pairwise-intersect.htm) tool.

Unlike what I did for QGIS, I will not show the GUI forms but the code that ArcGIS executes.

For the `in_features` parameter, we provide our two layers, `line` and `base`. We know we will get points, so we declare the output type as `POINT`.

```python
arcpy.analysis.PairwiseIntersect(
    in_features="line;base",
    out_feature_class=r"C:\Users\xxx\AppData\Local\Temp\ArcGISProTemp37912\Sans titre\Default.gdb\line_PairwiseIntersect",
    join_attributes="ALL",
    cluster_tolerance=None,
    output_type="POINT"
)
```

I’ll skip the steps for extracting WKB and WKT; here are the results:

- 0104000000020000000101000000e034efc8c83c3e4120166a8166d55341010100000040a4df9e8f3c3e416054525379d55341
- MultiPoint ((1981640.78490000218153 5199258.02210000157356262),(1981583.62060000002384186 5199333.30189999938011169))

ArcGIS gives us a slightly different result. Let's test with the other intersection tool: [Intersect](https://pro.arcgis.com/en/pro-app/latest/tool-reference/analysis/intersect.htm).

```python
arcpy.analysis.Intersect(
    in_features="line #;base #",
    out_feature_class=r"C:\Users\xxx\AppData\Local\Temp\ArcGISProTemp37912\Sans titre\Default.gdb\line_Intersect1",
    join_attributes="ALL",
    cluster_tolerance=None,
    output_type="POINT"
)
```

- 01040000000200000001010000008016d99e8f3c3e416054525379d553410101000000e034efc8c83c3e4120166a8166d55341
- MultiPoint ((1981583.62049999833106995 5199333.30189999938011169),(1981640.78490000218153 5199258.02210000157356262))

Again, a slightly different result but close to what we obtain with other GIS systems. What is happening here?

ArcGIS uses a concept sometimes found in open-source GIS: resolution and tolerance.
We can modify it by passing XY values as parameters. Here are the queries and results with a value of 0.00001 mm.

```python
with arcpy.EnvManager(XYResolution="0.00001 Millimeters", XYTolerance="0.00001 Millimeters"):
    arcpy.analysis.PairwiseIntersect(
        in_features="line;base",
        out_feature_class=r"C:\Users\xxx\AppData\Local\Temp\ArcGISProTemp37912\Sans titre\Default.gdb\line_PairwiseIntersect1",
        join_attributes="ALL",
        cluster_tolerance=None,
        output_type="POINT"
    )
```

- MultiPoint ((1981640.78490600734949112 5199258.02208840474486351),(1981583.6205737441778183 5199333.30187807604670525))
- 0104000000020000000101000000a099efc8c83c3e417ce5698166d553410101000000c0ebdd9e8f3c3e416cf8515379d55341

Ah, we get closer to our expected results! At least the gap has been reduced.

The equivalent of our select by location is done as follows:

```python
arcpy.management.SelectLayerByLocation(
    in_layer="line_PairwiseIntersect1;pairwiseIntersect_tolerance;lineIntersect",
    overlap_type="INTERSECT",
    select_features="line",
    search_distance=None,
    selection_type="NEW_SELECTION",
    invert_spatial_relationship="NOT_INVERT"
)
```

With a search distance (tolerance):

```python
arcpy.management.SelectLayerByLocation(
    in_layer="line_PairwiseIntersect1;pairwiseIntersect_tolerance;lineIntersect",
    overlap_type="INTERSECT",
    select_features="line",
    search_distance="0.000000001 Millimeters",
    selection_type="NEW_SELECTION",
    invert_spatial_relationship="NOT_INVERT"
)
```

In both cases, ArcGIS selects the intersection points. This is a good point for them.

#### Using WKB and ArcPy

In the next section, we will see how this works by using the basic functions directly.

```python
import binascii
# Import the base WKB
base = arcpy.FromWKB(binascii.unhexlify('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341'))
# Import the line WKB
line = arcpy.FromWKB(binascii.unhexlify('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341'))
# Calculate the intersection point in WKT
base.intersect(line, 1).WKT
# 'MULTIPOINT ((1981583.6207275391 5199333.3018798828), (1981640.7850952148 5199258.0220947266))'
# In WKB
base.intersect(line, 1).WKB
# bytearray(b'\x01\x04\x00\x00\x00\x02\x00\x00\x00\x01\x01\x00\x00\x00\x00\x00\xe8\x9e\x8f<>A\x00\x00RSy\xd5SA\x01\x01\x00\x00\x00\x00\x00\xfc\xc8\xc8<>A\x00\x00j\x81f\xd5SA')
# With conversion to display in hex
binascii.hexlify(base.intersect(line, 1).WKB)
# b'01040000000200000001010000000000e89e8f3c3e410000525379d5534101010000000000fcc8c83c3e4100006a8166d55341'
```

Now let's look at the spatial relationships between our `result` and the [geometries](https://pro.arcgis.com/en/pro-app/latest/arcpy/classes/geometry.htm) `base` and `line`.
We will use: disjoint, contains, crosses, equals, overlaps, touches, and within; the latter being our intersects.

Base:

```python
result.disjoint(base)
# False
```

```python
result.contains(base), result.crosses(base), result.equals(base), result.overlaps(base), result.touches(base), result.within(base)
# (False, False, False, False, False, True)
```

Line:

```python
result.disjoint(line)
# False
```

```python
result.contains(line), result.crosses(line), result.equals(line), result.overlaps(line), result.touches(line), result.within(line)
# (False, False, False, False, False, True)
```

We get the desired result. In fact, you may have noticed that ArcGIS does not offer a "strict" calculation like the others but something special. It is "tolerant."


## Algorithms and Code: How Does It Work?

### Disclaimer

This article targets all GIS professionals, regardless of their skill level. However, those already familiar with errors like 0.1 + 0.2 ≠ 0.3 might not find new information here. I aim to simplify these concepts. If you are interested in a more detailed version, please let me know, and I will consider creating one.

### How to Calculate If Point C Is on Segment AB? Calculate the Area of Triangle ABC!

In simpler terms, to determine if point \( C \) is on a line defined by points \( A \) and \( B \), we can use vector geometry. Here is a straightforward method:

1. **Coordinates of the Points**: Suppose the coordinates of points \( A \), \( B \), and \( C \) are \( (x_A, y_A) \), \( (x_B, y_B) \), and \( (x_C, y_C) \), respectively.

2. **Vectors AB and AC**: Calculate the vectors \( \overrightarrow{AB} \) and \( \overrightarrow{AC} \):
   - \( \overrightarrow{AB} = (x_B - x_A, y_B - y_A) \)
   - \( \overrightarrow{AC} = (x_C - x_A, y_C - y_A) \)

3. **Determinant**: Calculate the determinant of vectors \( \overrightarrow{AB} \) and \( \overrightarrow{AC} \). This determinant is given by:
   \[
   D = (x_B - x_A) \cdot (y_C - y_A) - (y_B - y_A) \cdot (x_C - x_A)
   \]

4. **Verification**:
   - If \( D = 0 \), then points \( A \), \( B \), and \( C \) are collinear, meaning \( C \) is on the line \( AB \).
   - If \( D \neq 0 \), then \( C \) is not on the line \( AB \).

### Example:

Suppose the coordinates are:
- \( A = (1, 2) \)
- \( B = (4, 6) \)
- \( C = (2, 3) \)

Calculate the vectors:
- \( \overrightarrow{AB} = (4 - 1, 6 - 2) = (3, 4) \)
- \( \overrightarrow{AC} = (2 - 1, 3 - 2) = (1, 1) \)

Calculate the determinant:
\[
D = 3 \cdot 1 - 4 \cdot 1 = 3 - 4 = -1
\]

Since \( D \neq 0 \), point \( C \) is not on the line \( AB \).

### Simple Python Implementation

```python
def orient2d(x_a, y_a, x_b, y_b, x_c, y_c):
    """
    Calculates the double of the area of the triangle formed by points A, B, and C.
    If the result is positive, the points are oriented counterclockwise.
    If the result is negative, the points are oriented clockwise.
    If the result is zero, the points are collinear.
    """
    return (x_b - x_a) * (y_c - y_a) - (y_b - y_a) * (x_c - x_a)

def shoelace_area(x_a, y_a, x_b, y_b, x_c, y_c):
    # Calculate the area using the shoelace formula
    area = 0.5 * (x_a * (y_b - y_c) + x_b * (y_c - y_a) + x_c * (y_a - y_b))
    return area

# Example usage
x_a, y_a = 1, 2
x_b, y_b = 4, 6
x_c, y_c = 2, 3

area = shoelace_area(x_a, y_a, x_b, y_b, x_c, y_c)
orient = orient2d(x_a, y_a, x_b, y_b, x_c, y_c)
print(f"The area of the triangle is: {area}")
print(f"The orientation of the triangle ABC is: {orient}")
```

The code returns a negative number, with the area being half of the orientation.

If we reverse \( A \) and \( B \) as follows:

```python
area = shoelace_area(x_b, y_b, x_a, y_a, x_c, y_c)
orient = orient2d(x_b, y_b, x_a, y_a, x_c, y_c)
print(f"The area of the triangle is: {area}")
print(f"The orientation of the triangle BAC is: {orient}")
```

We get a positive number. Point \( C \) is on the other side of axis \( AB \), meaning \( AB \) and \( C \) are not collinear.

```python
# Example with collinear points
x_a, y_a = 1, 1
x_b, y_b = 2, 2
x_c, y_c = 3, 3

area = shoelace_area(x_b, y_b, x_a, y_a, x_c, y_c)
orient = orient2d(x_b, y_b, x_a, y_a, x_c, y_c)
print(f"The area of the triangle is: {area}")
print(f"The orientation of the triangle BAC is: {orient}")
```

```python
# Example with collinear points
x_a, y_a = 0.1, 0.1
x_b, y_b = 0.2, 0.2
x_c, y_c = 0.3, 0.3

area = shoelace_area(x_b, y_b, x_a, y_a, x_c, y_c)
orient = orient2d(x_b, y_b, x_a, y_a, x_c, y_c)
print(f"The area of the triangle is: {area}")
print(f"The orientation of the triangle BAC is: {orient}")
```

Here, you should see a "joke." The area is not exactly zero but close to zero.

### Floating-Point Numbers

If we assume that \( C \) is the intersection point of a segment with \( AB \), then theoretically, the lines intersect at a well-defined point: \( C \). However, in practice, computers perform calculations using a finite numerical representation that can introduce small errors.

Computers predominantly use [floating-point representation](https://en.wikipedia.org/wiki/Floating_point) to store real numbers. This is also the norm for coordinates in our geometries. This famous "double" type is ubiquitous. However, its use can introduce rounding errors. I mentioned an operation earlier: 0.1 + 0.2 ≠ 0.3. Since 0.3 cannot be exactly represented, it leads to approximations. This is a well-known problem among computer scientists, to the point of having its own website: [https://0.30000000000000004.com/](https://0.30000000000000004.com/).
Similarly, I encourage reading the article [What Every Computer Scientist Should Know About Floating-Point Arithmetic](https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html).

### Accumulation of Errors

During complex calculations, these small rounding errors can accumulate and become significant.

When calculating the intersection of two lines, several arithmetic operations are required. Each operation can introduce a small error.

Solutions like GEOS or Clipper (for SAGA) rely on much more robust calculations than our example; they also use integers. The intersection calculation will be accurate—to the precision of the float.

### Precision Comparisons

In reality, what is missing in our GIS is tolerance.

When verifying if a point is on a line, the precision of the calculations can affect the result.

One rule when using floating-point numbers is to calculate based on a value. We should never directly compare a floating-point number. That is, avoid:

```python
0.1 + 0.2 == 0.3
```

Instead, use functions that perform approximate comparisons like:

```python
def compare_doubles(a, b, tolerance=1e-9):
    """
    Compare two floating-point numbers with a given tolerance.

    :param a: First number to compare
    :param b: Second number to compare
    :param tolerance: Tolerance for comparison (default 1e-9)
    :return: True if the numbers are equal within the tolerance, False otherwise
    """
    return abs(a - b) <= tolerance

# Example usage
a = 0.1 + 0.2
b = 0.3
tolerance = 1e-9

if compare_doubles(a, b, tolerance):
    print(f"The numbers {a} and {b} are equal with a tolerance of {tolerance}.")
else:
    print(f"The numbers {a} and {b} are not equal with a tolerance of {tolerance}.")
```

You should see this output:

`The numbers 0.30000000000000004 and 0.3 are equal with a tolerance of 1e-9.`

For a more in-depth study, I encourage reading the article [Comparing Two Floating-Point Numbers](https://embeddeduse.com/2019/08/26/qt-compare-two-floats/).

As you may have guessed, this is what

 ArcGIS does.
In many parts of our GIS, there are fuzzy comparisons. Perhaps future versions of GEOS will integrate this tolerance for relationships. :wink:

### Returning to SFCGAL

We saw that SFCGAL is the only library to give the correct result for `intersects`, but only in Python. With PostGIS, it returns false like GEOS. Why?

SFCGAL does not use floating-point numbers but a different arithmetic. To recall, our example code is:

```python
from pysfcgal import sfcgal
base = sfcgal.read_wkb('0102000000050000007997c6b68d3c3e4139eb62c260d55341ac9ea7316a3c3e41cbeb40e073d55341403e0bfbc33c3e41b3fc06f380d55341387a2a800c3d3e41f256b8176dd553417997c6b68d3c3e4139eb62c260d55341')
line = sfcgal.read_wkb('010200000002000000ea9c6d2b873c3e41a03d941b7cd5534133db7796ce3c3e413fba569864d55341')
result = base.intersection(line)
# Display WKT with 10 decimals
print(result.wktDecim(10))
print(base.intersects(result))
print(line.intersects(result))
```

We get the desired result. Now let's try to reproduce what happens in PostGIS.
In our query, we calculate the intersection point with `CG_Intersection`. At the end of its process, it converts its geometry and numbers to [PostGIS double](https://github.com/postgis/postgis/blob/d29ba84cb05988ab0aa1b2da3eef90108dfae1db/sfcgal/lwgeom_sfcgal.c#L559). This step is the cause of the problem. This conversion is equivalent to this in Python:

```python
# Create a point np using floats
p = sfcgal.lib.sfcgal_geometry_collection_geometry_n(result._geom, 0)
# Convert the gmp number to double
x = sfcgal.lib.sfcgal_point_x(p)
y = sfcgal.lib.sfcgal_point_y(p)
# Create the point with these doubles
np = sfcgal.Point(x, y)
print(base.intersects(np))
print(line.intersects(np))
```

And there it is, the "imprecision" of doubles gives us the wrong result.

A solution, which is not elegant and thus not yet implemented, would be to have functions that chain and do not continuously switch back and forth between numbers. With a `CG_IntersectsIntersection` function like this, the result in PostGIS would be correct.

```c
PG_FUNCTION_INFO_V1(sfcgal_intersects_intersection);
Datum
sfcgal_intersects_intersection(PG_FUNCTION_ARGS)
{
       GSERIALIZED *input0, *input1;
       GSERIALIZED *output;
       LWGEOM *lwgeom;
       bool intersect1, intersect2;
       char *values[3];

       sfcgal_geometry_t *geom0, *geom1;
       sfcgal_geometry_t *sfcgal_result;
       srid_t srid;

       sfcgal_postgis_init();

       input0 = PG_GETARG_GSERIALIZED_P(0);
       srid = gserialized_get_srid(input0);
       input1 = PG_GETARG_GSERIALIZED_P(1);
       geom0 = POSTGIS2SFCGALGeometry(input0);
       PG_FREE_IF_COPY(input0, 0);
       geom1 = POSTGIS2SFCGALGeometry(input1);
       PG_FREE_IF_COPY(input1, 1);

       sfcgal_result = sfcgal_geometry_intersection(geom0, geom1);
       output = SFCGALGeometry2POSTGIS(sfcgal_result, 0, srid);
       lwgeom = lwgeom_from_gserialized(output);
       intersect1 = sfcgal_geometry_intersects(geom0, sfcgal_result);
       intersect2 = sfcgal_geometry_intersects(geom1, sfcgal_result);
       values[0] = intersect1 ? "t" : "f";
       values[1] = intersect2 ? "t" : "f";
       values[2] = lwgeom_to_hexwkb_buffer(lwgeom, WKB_EXTENDED);
       lwpgnotice(
           "%s %s %s",
           values[0], values[1], values[2]);

       sfcgal_geometry_delete(geom0);
       sfcgal_geometry_delete(geom1);

       sfcgal_geometry_delete(sfcgal_result);

       PG_RETURN_POINTER(intersect1 && intersect2);
}
```

More generally, you can run this [mini Python script](https://github.com/lbartoletti/lbartoletti.github.io/blob/e2a7c896516af05da86d079c589edda415471e83/assets/2024_intersection_intersects/data/intersects_intersection_numbers.py) that summarizes what we have seen.

It performs our intersects/intersection calculation on two segments using Python's floats, Decimal, and Fraction.

### Bibliography

In addition to the documentation links mentioned occasionally, here are some links to supplement or deepen the topic.

First, credit where credit is due: this series of articles is just an illustrated explanation of the [JTS FAQ](https://locationtech.github.io/jts/jts-faq.html), which is the origin of GEOS. But since no one reads the documentation :wink:, let's put the link [here](https://locationtech.github.io/jts/jts-faq.html#D8).

On the robustness of segment intersection, you can start with this [Stack Exchange discussion](https://cs.stackexchange.com/questions/119760/robust-two-lines-segments-intersection-point-in-2d) and then read and use Shewchuk's code on robust calculations with floating-point numbers:
- https://people.eecs.berkeley.edu/~jrs/meshpapers/robnotes.pdf
- https://www.cs.cmu.edu/~quake/robust.html
- https://www.cs.cmu.edu/afs/cs/project/quake/public/code/predicates.c

Modern versions have been revisited, such as https://github.com/mourner/robust-predicates, which also provides a comprehensive article on this concept.

JTS/GEOS uses a modified version of these calculations. To avoid lengthening an already lengthy article, I have not shown examples with these functions, but the result is the same.

In our GIS, here are some links to the source codes used:
**QGIS**

- The functions for geometric calculations in [QGIS](https://github.com/qgis/QGIS/blob/0c41c22343ded7c6b6a7be0d382477128e837bd9/src/core/geometry/qgsgeometryutils_base.cpp).

**GRASS**

- The [v.clean manual](https://grass.osgeo.org/grass83/manuals/v.clean.html), which uses a `split` function that will split the segment with a [distance](https://github.com/OSGeo/grass/blob/9cb4745b6c4abfeaf542ef05468060d68af72703/vector/v.clean/split.c). To be exhaustive, I could have mentioned that this process can slightly modify the geometries.

**SAGA**

- The intersection calculation in [SAGA](https://github.com/saga-gis/saga-gis/blob/0e66e5a768d771052553f270c0ffe24efda1d0a8/saga-gis/src/saga_core/saga_api/geo_functions.cpp#L280).
- The [line crossings](https://github.com/saga-gis/saga-gis/blob/0e66e5a768d771052553f270c0ffe24efda1d0a8/saga-gis/src/tools/shapes/shapes_lines/line_crossings.cpp#L226) function.
- And the [Clipper2 library](https://github.com/AngusJohnson/Clipper2) used by SAGA.

**GEOS**

- The [intersects test](https://github.com/libgeos/geos/blob/a8d2ed0aba46f88f9b8987526e68eea6565d16ae/src/algorithm/LineIntersector.cpp#L222).
- The [orient2d calculation](https://github.com/libgeos/geos/blob/a8d2ed0aba46f88f9b8987526e68eea6565d16ae/src/algorithm/CGAlgorithmsDD.cpp#L54), where you can see an initial quick test and, if the robustness is not good enough, switch to another [arithmetic](https://github.com/libgeos/geos/blob/a8d2ed0aba46f88f9b8987526e68eea6565d16ae/include/geos/math/DD.h).

## How to Stop Overthinking and Live a Better Life!

We often encounter questions about the "irregularities" faced during common GIS operations: Why do snap operations in QGIS not always position exactly on the geometry? Why do overlay calculations lack precision? And why can the results be inconsistent?

Instead of getting lost in the quest for ultra-precision, here are some tips to improve your GIS experience and live a better life:

### Stop Chasing Ultra-Precision

*Rigorous precision at all costs can become a source of frustration. Accept that slight imprecision is inevitable and focus on what really matters.*

We live in an infinite world with finite resources. A few rounding errors won't hurt. How many decimal places do you really need? Is your precision decimeter, centimeter, millimeter, or beyond? How many approximations do you make daily while being precise? Is it exactly 21:02 or just 21:00? When you plan a trip to a QGIS meeting, are you precise to the second, minute, or quarter-hour? In short, precision depends on your context, and you'll rarely need to go below 10^-3 in Cartesian coordinates or 10^-8 in geodesic coordinates.

![Coordinate Precision](https://imgs.xkcd.com/comics/coordinate_precision.png)

### Manage Tolerance

*Use appropriate tolerances in your calculations to minimize the effects of rounding errors. Define tolerances suited to the scale and objectives of your project.*

In connection with the number of decimal places, you can also add a tolerance. In France, network managers know that their networks are characterized by three classes: A, B, or C (10 cm, 40 cm, etc.). A good practice is to consider if a point is within about X cm. In PostGIS, this can be characterized by using ST_DWithin rather than ST_Intersects.

### Use Topology

*Topology helps manage spatial relationships and correct geometric errors. Topological tools ensure that spatial entities adhere to certain rules, improving data consistency.*

If you really want nodes to be identical, topology is there for you. But be cautious; it slightly transforms the input data. Additionally, depending on the tools you use, it might not be respected when editing in other tools. Hence, the idea of shifting intelligence to the database: Thick database.

### Understand Different Types of Numbers

*Understand how numbers are represented in computers. This helps anticipate and manage calculation errors, especially differences between floating-point numbers and others.*

It's all the fault of floating-point numbers! You can use other tools, but be cautious; conversion can introduce errors.

### Invest in Tool Development

*Support the development and improvement of GIS tools, especially open-source projects like those from OSGeo!*

It can't be said enough: if a feature or bug bothers you, fund us! We will be happy to address it. This also applies to feature requests.

### Conclusion

*The quest for numerical perfection in GIS can be frustrating. By adopting a pragmatic approach and understanding the limits of numerical calculations, you can reduce stress and improve efficiency. Accepting these realities allows you to stop overthinking and start living a better, more serene, and productive life in your geospatial projects.*

Isn't it beautiful?

In reality, live your GIS as you wish, but be aware of how they function. Yes, their, because each can give you slightly different results.

I hope you found this series of articles interesting. More comparisons between tools are forthcoming.

And finally, thanks to my reviewers aaa, bbb, ccc, and xxx.
