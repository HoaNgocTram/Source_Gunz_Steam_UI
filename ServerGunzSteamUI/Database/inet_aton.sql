USE [GunzDB]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER OFF
GO

CREATE OR ALTER FUNCTION [dbo].[inet_aton] (@IP VARCHAR(15))
RETURNS BIGINT
AS
BEGIN
    DECLARE @A BIGINT, @B BIGINT, @C BIGINT, @D BIGINT
    DECLARE @iBegin INT, @iEnd INT
    
    SELECT @iBegin = 1
    SELECT @iEnd = CHARINDEX('.', @IP)
    SELECT @A = CAST(SUBSTRING(@IP, @iBegin, @iEnd - @iBegin) AS BIGINT)
    
    SELECT @iBegin = @iEnd + 1
    SELECT @iEnd = CHARINDEX('.', @IP, @iBegin)
    SELECT @B = CAST(SUBSTRING(@IP, @iBegin, @iEnd - @iBegin) AS BIGINT)
    
    SELECT @iBegin = @iEnd + 1
    SELECT @iEnd = CHARINDEX('.', @IP, @iBegin)
    SELECT @C = CAST(SUBSTRING(@IP, @iBegin, @iEnd - @iBegin) AS BIGINT)
    
    SELECT @iBegin = @iEnd + 1
    SELECT @D = CAST(SUBSTRING(@IP, @iBegin, LEN(@IP)) AS BIGINT)
    
    DECLARE @IPNumber BIGINT
    SELECT @IPNumber = @A * 16777216 + @B * 65536 + @C * 256 + @D
    
    RETURN @IPNumber
END
GO
