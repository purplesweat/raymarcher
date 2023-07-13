#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>

const int width = 480;
const int height = 360;
const double res = 0.25;

const double base_ambience = 0.1;
const double base_shine = 50;
const double base_diffuse = 1;
const double base_specular = 1;

double color, saturation, brightness;


typedef struct {
    double r, g, b;
} RGB;

typedef struct {
    double x, y, z;
} Point;

RGB* get_rgb() {
    RGB* out = malloc(sizeof(RGB));
    int hue = (int) (color * 3.6);
    if (saturation == 0) {
        out->r = brightness;
        out->g = brightness;
        out->b = brightness;
        return out;
    }
    int i;
    double f, p, q, t;
    if (hue == 360)
        hue = 0;
    else hue /= 60;

    i = trunc(hue);
    f = (double) hue - i;

    p = brightness * (1 - saturation);
    q = brightness * (1 - (saturation * f));
    t = brightness * (1 - (saturation * (1 - f)));

    switch (i) {
        case 0:
            out->r = brightness;
            out->g = t;
            out->b = p;
            break;
        case 1:
            out->r = q;
            out->g = brightness;
            out->b = p;
            break;
        case 2:
            out->r = p;
            out->g = brightness;
            out->b = t;
            break;
        case 3:
            out->r = p;
            out->g = q;
            out->b = brightness;
            break;
        case 4:
            out->r = t;
            out->g = p;
            out->b = brightness;
            break;
        default:
            out->r = brightness;
            out->g = p;
            out->b = q;
            break;
    }
    return out;
}

static void draw_pixel(cairo_t* cr, double x, double y) {
    cairo_rectangle(cr, x + width/2 + res/2,
            height - (y + height/2) + res/2,
            res, res);
    cairo_fill(cr);
}

static double length(double x, double y, double z) {
    return sqrt(x*x + y*y + z*z);
}

static double ptlength(Point* pt) {
    return length(pt->x, pt->y, pt->z);
}

static double divptlen(Point* pt) {
    double len = ptlength(pt);
    pt->x /= len;
    pt->y /= len;
    pt->z /= len;
    return len;
}

static void calc_sphere(double xpos, double ypos, double zpos,
        double radius, double obj_color, double x, double y, double z,
        double* SDE) {
    double dist = length(xpos-x, ypos-y, zpos-z) - radius;
    if (dist < *SDE) {
        *SDE = dist;
        color = obj_color;
    }
}

static void calc_plane(double normalX, double normalY, double normalZ,
        double distance, double obj_color, double x, double y, double z,
        double* SDE) {
    double dist = x * normalX + y * normalY + z * normalZ + distance;
    if (dist < *SDE) {
        *SDE = dist;
        color = obj_color;
    }
}

static void calc_sdes(double x, double y, double z, double* SDE) {
    calc_plane(0, 1, 0, 50, 0, x, y, z, SDE);
    calc_sphere(-30, -10, 20, 70, 40, x, y, z, SDE);
    calc_sphere(50, 20, 30, 50, 70, x, y, z, SDE);
}

static void calc_intersections(int max_length, Point* center,
        Point* ray, double* raylength, double* SDE) {
    *SDE = 10000;
    *raylength = 1;
    while (*SDE >= 0.01 && *raylength <= max_length) {
        calc_sdes(center->x + ray->x * *raylength,
                center->y + ray->y * *raylength,
                center->z + ray->z * *raylength, SDE);
        *raylength += *SDE;
    }
}

Point* get_normal(Point* pt, double* SDE) {
    Point* normal = malloc(sizeof(Point));

    *SDE = 10000;
    calc_sdes(pt->x + 0.01, pt->y, pt->z, SDE);
    normal->x = *SDE;
    *SDE = 10000;
    calc_sdes(pt->x - 0.01, pt->y, pt->z, SDE);
    normal->x -= *SDE;

    *SDE = 10000;
    calc_sdes(pt->x, pt->y + 0.01, pt->z, SDE);
    normal->y = *SDE;
    *SDE = 10000;
    calc_sdes(pt->x, pt->y - 0.01, pt->z, SDE);
    normal->y -= *SDE;

    *SDE = 10000;
    calc_sdes(pt->x, pt->y, pt->z + 0.01, SDE);
    normal->z = *SDE;
    *SDE = 10000;
    calc_sdes(pt->x, pt->y, pt->z - 0.01, SDE);
    normal->z -= *SDE;

    divptlen(normal);

    return normal;
}

static void raymarch_point(cairo_t* cr, double x, double y) {
    double raylength, SDE;
    Point* center = malloc(sizeof(Point));
    Point* ray = malloc(sizeof(Point));

    center->x = 0; center->y = 0; center->z = -100;
    ray->x = x, ray->y = y, ray->z = -center->z;
    divptlen(ray);
    calc_intersections(1000, center, ray, &raylength, &SDE);

    if (SDE < 0.01) {
        Point* intersect = malloc(sizeof(Point));
        intersect->x = center->x + ray->x * raylength;
        intersect->y = center->y + ray->y * raylength;
        intersect->z = center->z + ray->z * raylength;

        Point* normal = get_normal(intersect, &SDE);

        Point* light = malloc(sizeof(Point));
        light->x = 20 - intersect->x;
        light->y = 50 - intersect->y;
        light->z = -100 - intersect->z;
        double len = divptlen(light);

        double diffuse = normal->x * light->x +
            normal->y * light->y + normal->z * light->z;
        double dot = normal->x * ray->x +
            normal->y * ray->y + normal->z * ray->z;

        Point* ref = malloc(sizeof(Point));
        ref->x = 2 * dot * normal->x - ray->x;
        ref->y = 2 * dot * normal->y - ray->y;
        ref->z = 2 * dot * normal->z - ray->z;
        double specular = - ref->x * light->x -
                ref->y * light->y - ref->z * light->z;
        specular = (specular < 0) ? 0
            : base_specular * pow(10, log10(specular) * base_shine);
        diffuse *= base_diffuse;

        calc_intersections(len, intersect, light, &raylength, &SDE);
        double shade = specular + diffuse + base_ambience;

        saturation = (specular > 0.7) ? 1 - specular
            : 1;
        shade -= specular;

        brightness = (raylength >= len - 1) ? shade : base_ambience;
        
        free(ref);
        free(light);
        free(normal);
        free(intersect);
    } else {
        brightness = 0;        
    }
    RGB* rgb = get_rgb();
    cairo_set_source_rgb(cr, rgb->r, rgb->g, rgb->b);
    draw_pixel(cr, x, y);

    free(rgb);
    free(ray);
    free(center);
}

static void do_drawing(cairo_t *cr)
{
    for (double x = -width/2; x <= width/2; x+=res) {
        for (double y = -height/2; y <= height/2; y+=res) {
            raymarch_point(cr, x, y);
        }
    }
}


/* GTK wrapper code */

static gboolean draw_func(GtkDrawingArea* da, cairo_t* cr,
        int width, int height, gpointer user_data) {
    do_drawing(cr);

    return false;
}

static void on_activate(GtkApplication* app) {
    GtkWidget* window = gtk_application_window_new(app);
    
    GtkWidget* da = gtk_drawing_area_new();
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(da), width);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(da), height);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(da),
            (GtkDrawingAreaDrawFunc)draw_func, NULL, NULL);

    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
    gtk_window_set_child(GTK_WINDOW(window), da);
    gtk_window_set_title(GTK_WINDOW(window), "Raymarching");
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char* argv[]) {
    GtkApplication* app = gtk_application_new("com.example.GtkApplication",
            G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}
