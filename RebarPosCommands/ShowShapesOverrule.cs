﻿using System;
using System.Collections.Generic;
using Autodesk.AutoCAD.ApplicationServices;
using Autodesk.AutoCAD.Runtime;
using Autodesk.AutoCAD.DatabaseServices;
using Autodesk.AutoCAD.Geometry;
using Autodesk.AutoCAD.GraphicsInterface;
using Autodesk.AutoCAD.Colors;
using OZOZ.RebarPosWrapper;

namespace RebarPosCommands
{
    public class ShowShapesOverrule : DrawableOverrule
    {
        private static ShowShapesOverrule mInstance = new ShowShapesOverrule();
        private Dictionary<string, bool> mIds;

        public static ShowShapesOverrule Instance { get { return mInstance; } }

        public ShowShapesOverrule()
        {
            mIds = new Dictionary<string, bool>();
            SetCustomFilter();
        }

        public void Add()
        {
            string key = HostApplicationServices.WorkingDatabase.FingerprintGuid;
            if (!mIds.ContainsKey(key))
                mIds.Add(key, false);
        }

        public void Remove()
        {
            mIds.Remove(HostApplicationServices.WorkingDatabase.FingerprintGuid);
        }

        public bool Has()
        {
            return mIds.ContainsKey(HostApplicationServices.WorkingDatabase.FingerprintGuid);
        }

        public override bool IsApplicable(RXObject overruledSubject)
        {
            RebarPos pos = overruledSubject as RebarPos;
            Database db = pos.Database;
            if (db == null) return false;
            return (mIds.ContainsKey(db.FingerprintGuid));
        }

        public override bool WorldDraw(Drawable drawable, WorldDraw wd)
        {
            if (wd.RegenAbort || wd.IsDragging)
            {
                return base.WorldDraw(drawable, wd);
            }

            RebarPos pos = drawable as RebarPos;
            if (pos == null)
            {
                return base.WorldDraw(drawable, wd);
            }

            WorldGeometry g = wd.Geometry;
            SubEntityTraits s = wd.SubEntityTraits;

            // Get geometry
            Point3d pt = pos.BasePoint;
            Vector3d dir = pos.DirectionVector;
            Vector3d up = pos.UpVector;
            Vector3d norm = pos.NormalVector;

            double w = pos.Width / dir.Length;
            double h = pos.Height / dir.Length;

            // Draw shape
            PosShape.Shape[] shapes = pos.Shapes;
            if (shapes.Length != 0)
            {
                // Get extents
                double xmin = double.MaxValue, ymin = double.MaxValue, xmax = double.MinValue, ymax = double.MinValue;
                foreach (PosShape.Shape shape in shapes)
                {
                    if (shape is PosShape.ShapeLine)
                    {
                        PosShape.ShapeLine line = shape as PosShape.ShapeLine;
                        xmin = Math.Min(xmin, Math.Min(line.X1, line.X2));
                        ymin = Math.Min(ymin, Math.Min(line.Y1, line.Y2));
                        xmax = Math.Min(xmax, Math.Max(line.X1, line.X2));
                        ymax = Math.Min(ymax, Math.Max(line.Y1, line.Y2));
                    }
                    else if (shape is PosShape.ShapeArc)
                    {
                        PosShape.ShapeArc arc = shape as PosShape.ShapeArc;
                        xmin = Math.Min(xmin, arc.X - arc.R);
                        xmax = Math.Max(xmax, arc.X + arc.R);
                        ymin = Math.Min(ymin, arc.Y - arc.R);
                        ymax = Math.Max(ymax, arc.Y + arc.R);

                    }
                    else if (shape is PosShape.ShapeText)
                    {
                        PosShape.ShapeText text = shape as PosShape.ShapeText;
                        xmin = Math.Min(xmin, text.X - text.Height);
                        xmax = Math.Max(xmax, text.X + text.Height);
                        ymin = Math.Min(ymin, text.Y - text.Height);
                        ymax = Math.Max(ymax, text.Y + text.Height);

                    }
                }

                // Scale
                double xscale = 0.9 * w / (xmax - xmin);
                double yscale = 0.9 * h / (ymax - ymin);
                double scale = Math.Min(xscale, yscale);
                // Client offsets
                double xoff = (w - scale * (xmax - xmin)) / 2.0;
                double yoff = 2.0 * h;//(h - scale * (ymax - ymin)) / 2.0;

                // Transform
                Matrix3d trans = Matrix3d.AlignCoordinateSystem(new Point3d(xmin, ymin, 0), Vector3d.XAxis / scale, Vector3d.YAxis / scale, Vector3d.ZAxis / scale, pt + dir * xoff + up * yoff, dir, up, norm);
                g.PushModelTransform(trans);

                // Draw shapes
                foreach (PosShape.Shape shape in shapes)
                {
                    s.Color = shape.Color.ColorIndex;
                    if (shape is PosShape.ShapeLine)
                    {
                        PosShape.ShapeLine line = shape as PosShape.ShapeLine;
                        Point3dCollection poly = new Point3dCollection();
                        poly.Add(new Point3d(line.X1, line.Y1, 0.0));
                        poly.Add(new Point3d(line.X2, line.Y2, 0.0));
                        g.Polyline(poly, Vector3d.ZAxis, IntPtr.Zero);
                    }
                    else if (shape is PosShape.ShapeArc)
                    {
                        PosShape.ShapeArc arc = shape as PosShape.ShapeArc;

                    }
                    else if (shape is PosShape.ShapeText)
                    {
                        PosShape.ShapeText text = shape as PosShape.ShapeText;
                    }
                }

                // Reset transform
                g.PopModelTransform();
            }

            return base.WorldDraw(drawable, wd);
        }
    }
}