// Portions adapted from Emgu.  Those portions are:
//----------------------------------------------------------------------------
//  Copyright (C) 2004-2020 by EMGU Corporation. All rights reserved.       
//----------------------------------------------------------------------------
//
// GNU GENERAL PUBLIC LICENSE
// Version 3, 29 June 2007

using Emgu.TF.Lite;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Darwin.ML
{
    /// <summary>
    /// Uses Emgu.TF.Lite to load and run inference with a TensorFlow Lite model.
    /// 
    /// </summary>
    public class MLModel : IDisposable
    {
        private Interpreter _interpreter;
        private FlatBufferModel _model = null;
        private string[] _labels = null;
        private Tensor _inputTensor;
        private Tensor _outputTensor;

        public MLModel(string modelFilename)
        {
            if (modelFilename == null)
                throw new ArgumentNullException(nameof(modelFilename));

            if (!File.Exists(modelFilename))
                throw new Exception(modelFilename + " does not exist.");

            var _model = new FlatBufferModel(modelFilename);

            if (!_model.CheckModelIdentifier())
                throw new Exception("Model identifier check failed");

            _interpreter = new Interpreter(_model);

            Status allocateTensorStatus = _interpreter.AllocateTensors();

            if (allocateTensorStatus == Status.Error)
                throw new Exception("Failed to allocate tensor");

            if (_inputTensor == null)
            {
                int[] input = _interpreter.InputIndices;
                _inputTensor = _interpreter.GetTensor(input[0]);
            }

            if (_outputTensor == null)
            {
                int[] output = _interpreter.OutputIndices;
                _outputTensor = _interpreter.GetTensor(output[0]);
            }
        }

        public float[] Run(float[] grayscaleImage)
        {
            if (grayscaleImage == null)
                throw new ArgumentNullException(nameof(grayscaleImage));

            Marshal.Copy(grayscaleImage, 0, _inputTensor.DataPointer, grayscaleImage.Length);

            _interpreter.Invoke();

            float[] coordinates = _outputTensor.Data as float[];

            if (coordinates == null)
                return null;

            return coordinates;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            //if (disposing)
            //{
            //    // Free managed resources
            //}
            
            // Free unmanaged resources
            if (_interpreter != null)
            {
                _interpreter.Dispose();
                _interpreter = null;
            }

            if (_model != null)
            {
                _model.Dispose();
                _model = null;
            }
        }
    }
}
